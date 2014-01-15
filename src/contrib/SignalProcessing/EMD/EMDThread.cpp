#include "PCHIncludes.h" // Make the compiler's Pre-Compiled Headers feature happy
#pragma hdrstop

#include "EMDThread.h"

#include "MeasurementUnits.h"
#include "BCIError.h"
#include <vector>
#include <cmath>

using namespace std;

void EMDThread::OnPublish() const {
	BEGIN_PARAMETER_DEFINITIONS
		"Filtering float SiftStopStdDev= 0.3"
			" 0.3 0.05 1.0 // Standard deviation used to stop EMD sifting",
		"Filtering int Interpolation= 1"
			" 1 0 1 // Interpolation method to use for calculating bounding envelopes (0 = linear, 1 = cubic)",
		"Filtering int MaxIMFs= 10"
			" 10 0 64 // Maximum number of Intrinsic Mode Functions (IMFs) to return",
	END_PARAMETER_DEFINITIONS
}

void EMDThread::OnPreflight( const SignalProperties& Input,
                                 SignalProperties& Output ) const {
	PreflightCondition( Parameter( "SiftStopStdDev" ) >= 0.05 );
	PreflightCondition( Parameter( "SiftStopStdDev" ) <= 1.0 );

	switch( ( int )Parameter( "Interpolation" ) ) {
		case linear:
		case cubic:
			break;
		default:
			bcierr << "Unsupported Interpolation value" << endl;
	}

	PreflightCondition( Parameter( "MaxIMFs" ) >= 0 );
	PreflightCondition( Parameter( "MaxIMFs" ) <= 64 );

	// Expand the number of channels to allow storage of all IMFs
	Output = Input;
	Output.SetChannels( Output.Channels() * ( int ) Parameter( "MaxIMFs" ) );
	
	// Give the IMF channels some reasonable labels
	for ( int i = 0; i < Input.ChannelLabels().Size(); i++ ) {
		for ( int j = 0; j < ( int ) Parameter( "MaxIMFs" ); j++ ) {
			Output.ChannelLabels()[i * ( int ) Parameter( "MaxIMFs" ) + j] = Input.ChannelLabels()[i];
		}
	}

	// Make sure the visualizer (if enabled) draws all the channels and elements
    Output.ElementUnit().SetRawMin( 0 ).SetRawMax( Output.Elements() - 1 );
    Output.ChannelUnit().SetRawMin( 0 ).SetRawMax( Output.Channels() - 1 );
}

void EMDThread::OnInitialize( const SignalProperties& Input,
                            const SignalProperties& Output ) {
	siftStopSD = Parameter( "SiftStopStdDev" );
	envelopeInterpolation = ( interpolationMethod ) ( int ) Parameter( "Interpolation" );
	maxIMFs = Parameter( "MaxIMFs" );
}

void EMDThread::OnProcess( const GenericSignal& Input, GenericSignal& Output ) {
	// Zero out the output
	for ( unsigned int chInd = 0; chInd < Channels().size(); chInd++ ) {
		for ( int sample = 0; sample < Output.Elements(); sample++ ) {
			Output( Channels()[chInd], sample ) = 0.0;
		}
	}

	for ( unsigned int chInd = 0; chInd < Channels().size(); chInd++ ) {
		int channel = Channels()[chInd];

		// Copy the signal into the Output as the residue
        int residue = maxIMFs * channel;
		for ( int sample = 0; sample < Input.Elements(); sample++ ) {
			Output( residue, sample ) = Input( channel, sample );
		}

		// Decompose the signal (maxIMFs - 1) times or until the signal becomes monotonic
		for ( ; residue < maxIMFs * ( channel + 1 ) - 2; residue++ ) {
			if ( !SingleEMD( Output, residue ) ) {
				break;
			}
		}

		// Sanity check
		// Add the EMD-ed outputs together and compare it to the input
		//     They should be within floating-point error of each other
		std::vector<double> check( Input.Elements(), 0 );
		for ( int i = 0 ; i < maxIMFs; i++ ) {
			for ( int sample = 0; sample < Input.Elements(); sample++ ) {
				check[sample] += Output( maxIMFs * channel + i, sample );
			}
		}
		for ( int sample = 0; sample < Input.Elements(); sample++ ) {
			check[sample] -= Input( channel, sample );
			if ( abs( check[sample] ) > 0.01 ) {
				bcierr << "Value (" << channel << ", " << sample << ") = " << check[sample] << endl;
			}
		}
	}
}

/**
 * Takes a GenericSignal and the channel holding the data to perform EMD on
 * The resulting Intrinsic Mode function will be placed in the given channel
 * The residue will be placed in the following channel
 *
 * Returns true if a further EMD can be performed on the residue
 *   based on monotonicity alone
 */
bool EMDThread::SingleEMD( GenericSignal& Output, const int channel ) {
	// Use a copy of the signal for sifting
	std::vector<double> previousSift( Output.Elements(), 0 );
	for ( int sample = 0; sample < Output.Elements(); sample++ ) {
		previousSift[sample] = Output( channel, sample );
	}

	// Sift the data until done
	std::vector<double> siftResult( Output.Elements(), 0 );
	bool canSift = true;
	while ( ( canSift = SingleSift( previousSift, &siftResult ) ) ) {
		// Calculate the change between the data and the sift
		double stdDev = 0.0;
		for ( unsigned int sample = 0; sample < previousSift.size(); sample++ ) {
			stdDev +=
				pow( previousSift[sample] - siftResult[sample], 2 )
				/ ( pow( previousSift[sample], 2 ) + EPSILON );
		}

		// Stop sifting if the SD cutoff is reached
		if ( stdDev < siftStopSD ) {
			break;
		}

		// Continue sifting
		previousSift = siftResult;
	}

	// Copy the IMF and residue into the GenericSignal
	for ( int sample = 0; sample < Output.Elements(); sample++ ) {
		// Residue
		Output( channel + 1, sample ) = Output( channel, sample ) - siftResult[sample];

		// IMF
		Output( channel, sample ) = siftResult[sample];
	}

	return canSift;
}

/**
 * Takes a vector containing the signal to be sifted
 *     and a vector to place the sifted signal (must have the same size)
 *
 * Returns true if a sift EMD can be performed on the residue
 *     The sifted signal will not be modified when false
 */
bool EMDThread::SingleSift( const std::vector<double> &stream, std::vector<double> *sifted ) {
	// Calculate the approximate derivative
	std::vector<double> deriv( stream.begin() + 1, stream.end() );
	for( unsigned int sample = 0; sample < stream.size() - 1; sample++ ) {
		deriv[sample] =- stream[sample];
	}

	// Find the indices of the local extrema
	std::list<int> extrema;
	for ( unsigned int d = 0; d < deriv.size() - 1; d++ ) {
		if ( deriv[d] == 0 ) {
			extrema.push_back( d );

			// Capture flat peaks/valleys
			if ( deriv[d + 1] != 0 ) {
				extrema.push_back( d + 1 );
			}

		} else if ( deriv[d] * deriv[d + 1] < 0 ) {
			// Define straddling zeros as d + 1
			extrema.push_back( d + 1 );
		}
	}

	// Separate the local extrema
	std::list<int> peaks;
	std::list<int> valleys;
	double previous = stream[0];
	bool exToggle = true;
	for ( std::list<int>::iterator iter = extrema.begin(); iter != extrema.end(); ++iter ) {
		double value = stream[*( iter )];
		if ( value != previous ) {
			exToggle = !exToggle;
		}
		exToggle ? peaks.push_back( *iter ) : valleys.push_back( *iter );
		previous = value;
	}

	// Make sure the signal is not monotonic
	if ( peaks.size() < 1 || valleys.size() < 1 ) {
		return false;
	}

	// Flip the peaks and valleys if necessary
	if ( stream[peaks.front()] < stream[valleys.front()] ) {
		std::list<int> temp = peaks;
		peaks = valleys;
		valleys = temp;
	}

	// Add the endpoints to each of the extrema lists to complete the indexed envelope
	if ( peaks.front() != 0 ) {
		peaks.push_front( 0 );
	}
	if ( peaks.back() != stream.size() - 1 ) {
		peaks.push_back( stream.size() - 1 );
	}
	if ( valleys.front() != 0 ) {
		valleys.push_front( 0 );
	}
	if ( valleys.back() != stream.size() - 1 ) {
		valleys.push_back( stream.size() - 1 );
	}

	// Interpolate over the envelope and sift the stream
	if ( envelopeInterpolation == linear ) {
		std::list<int>::iterator maxEnv = peaks.begin();
		std::list<int>::iterator minEnv = valleys.begin();
		int prevMax = *( maxEnv );
		int prevMin = *( minEnv );
		maxEnv++;
		minEnv++;
		for ( int i = 0; i < ( signed int )sifted->size(); i++ ) {
			//Increment the envelopes when necessary
			if ( i > *( maxEnv ) ) {
				prevMax = *( maxEnv );
				maxEnv++;
			}
			if ( i > *( minEnv ) ) {
				prevMin = *( minEnv );
				minEnv++;
			}

			double max = (i - *( maxEnv ) )
				* ( stream[*( maxEnv )] - stream[prevMax] )
				/ ( *( maxEnv ) - prevMax );
			double min = (i - prevMin )
				* ( stream[*( minEnv )] - stream[prevMin] )
				/ ( *( minEnv ) - prevMin );
			(*sifted)[i] = stream[i] - (max - min) / 2.0;
		}
	} else if ( envelopeInterpolation == cubic ) {
		//Although this could be parallelized,
		//  There's too much overhead from thread creation
		std::vector<double> maxEnv( stream.size(), 0 );
		CubicSplineInterpolate(
				std::vector<int>( peaks.begin(), peaks.end() ),
				stream,
				&maxEnv );

		std::vector<double> minEnv( stream.size(), 0 );
		CubicSplineInterpolate(
				std::vector<int>( valleys.begin(), valleys.end() ),
				stream,
				&minEnv );

		for ( unsigned int i = 0; i < sifted->size(); i++ ) {
			(*sifted)[i] = stream[i] - ( maxEnv[i] + minEnv[i] ) / 2.0;
		}
	}

	return true;
}

/**
 * Calculates the natural spline with the given (x, y) points
 */
void EMDThread::CubicSplineInterpolate( const std::vector<int> &x, const std::vector<double> &y, std::vector<double> *spline ) {
    // Tridiagonal matrix algorithm
	// See: https://en.wikipedia.org/wiki/Tridiagonal_matrix_algorithm#Method
	std::vector<double> a( x.size(), 0 ); // Sub diagonal
	std::vector<double> b( x.size(), 0 ); // Center diagonal
	std::vector<double> c( x.size(), 0 ); // Super diagonal
	// Column derived from y[x]
	//     To be transformed into the cubic coefficients
	std::vector<double> k( x.size(), 0 );


	// Initialize the values of the matrix equation
	// See: http://en.wikipedia.org/wiki/Spline_interpolation#Example
	int i = 0;
	a[i] = 0.0;
	c[i] = 1.0 / ( x[i + 1] - x[i] );
	b[i] = 2.0 * c[i];
	k[i] = 3.0 * ( y[x[i + 1]] - y[x[i]] ) * c[i] * c[i];
	for ( i = 1; i < ( signed int )x.size() - 1; i++ ) {
		a[i] = c[i - 1];
		c[i] = 1.0 / ( x[i + 1] - x[i] );
		b[i] = 2.0 * ( a[i] + c[i] );
		k[i] = 3.0 * ( ( y[x[i + 1]] - y[x[i]] ) * c[i] * c[i]
					+ ( y[x[i]] - y[x[i - 1]] ) * a[i] * a[i] );
	}
	i = x.size() - 1;
	a[i] = c[i - 1];
	c[i] = 0.0;
	b[i] = 2.0 * a[i];
	k[i] = 3.0 * ( y[x[i]] - y[x[i - 1]] ) * a[i] * a[i];

	// Forward sweep
	i = 0;
	c[i] = c[i] / b[i];
	k[i] = k[i] / b[i];
	for ( i = 1 ; i < ( signed int )x.size(); i++ ) {
		double factor = 1.0 / ( b[i] - a[i] * c[i - 1] );
		c[i] = c[i] * factor;
		k[i] = ( k[i] - a[i] * k[i - 1] ) * factor;
	}

	// Back substitution
	for ( i = x.size() - 2 ; i >= 0; i-- ) {
		k[i] = k[i] - c[i] * k[i + 1];
	}

	// Calculate the natural spline from 0 to y.size()
	//   'spline' is assumed to be a vector of size y.size()
	// This "nested" for loop is actually linear from 0 to (x.size()-1)
	i = 0;
	for ( int xit = 0; xit < ( signed int )x.size() - 1; xit++ ) {
		for ( ; i < x[xit + 1]; i++ ) {
			double xFac = ( double )( x[xit + 1] - x[xit] );
			double yFac = y[x[xit + 1]] - y[x[xit]];

			double t = ( double ) ( i - x[xit] ) / xFac;
			double aFac = k[xit] * xFac - yFac;
			double bFac = yFac - k[xit + 1] * xFac;

			(*spline)[i] = ( 1.0 - t ) * y[x[xit]]
					+ t * y[x[xit + 1]]
					+ t * ( 1.0 - t ) * ( aFac * ( 1.0 - t ) + bFac * t );
		}
	}
	(*spline)[y.size() - 1] = y[y.size() - 1];
}
