#include "PCHIncludes.h" // Make the compiler's Pre-Compiled Headers feature happy
#pragma hdrstop

#include "InstantFrequencyFilter.h"

#include "MeasurementUnits.h"
#include "BCIError.h"
#include <vector>
#include <cmath>

using namespace std;

RegisterFilter( InstantFrequencyFilter, 2.C );

InstantFrequencyFilter::InstantFrequencyFilter()
	: numChannels( 10 ), frequencyBandwidth( 0.5f ) {
  BEGIN_PARAMETER_DEFINITIONS
	"Filtering int NumChannels= 10"
		" 10 0 % // Number of channels to combine into a single channel", 
	"Filtering floatlist ValidFrequencies= 1 25"
		" % // Frequencies that will be kept", 
	"Filtering float FrequencyBand= 0.5"
		" 0.5 0 10 // Area around the valid frequencies to allow"
  END_PARAMETER_DEFINITIONS
}

InstantFrequencyFilter::~InstantFrequencyFilter() { }

void InstantFrequencyFilter::Preflight( const SignalProperties& Input,
                                 SignalProperties& Output ) const {
	PreflightCondition( Parameter( "NumChannels" ) >= 0 );
	PreflightCondition( Parameter( "NumChannels" ) <= Input.Channels() );
	
	PreflightCondition( Parameter( "ValidFrequencies" )->NumValues() > 0 );
	for ( int i = 0; i < Parameter( "ValidFrequencies" )->NumValues(); i++ ) {
		PreflightCondition( Parameter( "ValidFrequencies" )( i ) > 0 );
	}
	
	PreflightCondition( Parameter( "FrequencyBand" ) >= 0.0 );
	PreflightCondition( Parameter( "FrequencyBand" ) <= 10.0 );
	
	//Shrink the number of channels into channels within and without a few chosen frequencies
	Output = Input;
	Output.SetChannels( 3 * Input.Channels() / ( int ) Parameter( "NumChannels" ) );
}

void InstantFrequencyFilter::Initialize( const SignalProperties& Input,
                            const SignalProperties& Output ) {
	numChannels = Parameter( "NumChannels" );
	validFrequencies = std::vector<double>( Parameter( "ValidFrequencies" )->NumValues() );
	for ( int i = 0; i < ( signed int )validFrequencies.size(); i++ ) {
		validFrequencies[i] = Parameter( "ValidFrequencies" )( i );
	}
	frequencyBandwidth = Parameter( "FrequencyBand" );
}

void InstantFrequencyFilter::Process( const GenericSignal& Input, GenericSignal& Output ) {
	// Zero out the output
	for ( int channel = 0; channel < Output.Channels(); channel++ ) {
		for ( int sample = 0; sample < Output.Elements(); sample++ ) {
			Output( channel, sample ) = 0.0;
		}
	}
    
    // Put each sample into the correct output bucket
	for ( int channel = 0; channel < Input.Channels(); channel++ ) {
		bool isGood = false;

		// Copy the data into a vector
		std::vector<double> stream( Input.Elements(), 0 );
		for ( int sample = 0; sample < Input.Elements(); sample++ ) {
			stream[sample] = Input( channel, sample );
		}
		
		// Get the zero positions
		std::list<double> zeroList;
		RefinedGeneralizedZeroCrossing( stream, zeroList );

		// Need at least 2 points to calculate the frequency
		if ( zeroList.size() > 1 ) {
			std::vector<double> zeros( zeroList.begin(), zeroList.end() );
		
			// Calculate and add the whole, half, and quarter frequencies
			//   There are $n = zeros.size() - 1$ quarter periods
			//   Each   whole period is added in the following pattern: 1, 2, 3, 4, ... n - 3 times ... , 4, 3, 2, 1
			//   Each    half period is added in the following pattern: 0, 0, 1, 2, ... n - 1 times ... , 2, 1, 0, 0
			//   Each quarter period is added in the following pattern: 0, 0, 0, 1, ... n     times ... , 1, 0, 0, 0
			double frequency = 0.0;
			int samples = 0;

			// Enough information to calculate at least one instantaneous frequency
			if ( zeroList.size() > 7 ) {
				for ( int i = 0; i < 3; i++ ) {
					frequency += ( i + 1.0 ) / ( zeros[i + 4] - zeros[i] );
				}
				for ( int i = 3; i < ( signed int ) zeros.size() - 7; i++ ) {
					frequency += 
							  4.0 / ( zeros[i + 4] - zeros[i] )
							+ 2.0 / ( zeros[i + 2] - zeros[i] )
							+ 1.0 / ( zeros[i + 1] - zeros[i] );
				}
				for ( int i = zeros.size() - 7; i < ( signed int )zeros.size() - 4; i++ ) {
					frequency += ( zeros.size() - i - 4.0 ) / ( zeros[i + 4] - zeros[i] );
				}
				samples = 12 * ( zeros.size() - 7 );

			// Not quite enough information, so as much is used as possible
			} else {
				for ( int i = 0; i < ( signed int )zeros.size() - 4; i++ ) {
					frequency += 1.0 / ( zeros[i + 4] - zeros[i] );
				}
				for ( int i = 0; i < ( signed int )zeros.size() - 2; i++ ) {
					frequency += 0.5 / ( zeros[i + 2] - zeros[i] );
				}
				for ( int i = 0; i < ( signed int )zeros.size() - 1; i++ ) {
					frequency += 0.25 / ( zeros[i + 1] - zeros[i] );
				}
				samples = 3 * zeros.size() - 7;
			}

			// Check to see if the frequency falls into the valid range
			frequency *= Input.Properties().SamplingRate() / samples; 
			for ( int i = 0; i < ( signed int )validFrequencies.size(); i++ ) {
				if ( abs( frequency - validFrequencies[i] ) <= frequencyBandwidth ) {
					isGood = true;
					break;
				}
			}
		}
		
		// Add the signal into the appropriate channel
		//   The first channel holds signals falling into the buckets
		//   The second channel holds everything else
		int outChannel = isGood ? 3 * ( channel / numChannels ) : 3 * ( channel / numChannels ) + 1;
		for ( int sample = 0; sample < Input.Elements(); sample++ ) {
			Output( outChannel, sample) += Input( channel, sample );
		}
	}
    
    // Fill the third bucket with the whole signal
    for ( int channel = 0; channel < Output.Channels(); channel += 3 ) {
		for ( int sample = 0; sample < Output.Elements(); sample++ ) {
			Output( channel + 2, sample ) = Output( channel, sample ) + Output( channel + 1, sample );
		}
	}
}

/**
 * Returns (in a list) the estimated x coordinates 
 *     where the sampled signal crosses zero or hits a local extrema
 */
void InstantFrequencyFilter::RefinedGeneralizedZeroCrossing( 
		const std::vector<double>& stream, std::list<double>& zeros ) {
	for ( int i = 0; i < ( signed int )stream.size() - 2; i++ ) {
		double deriv = stream[i + 1] - stream[i];
		double nextDeriv = stream[i + 2] - stream[i + 1];

		// Capture flat extrema
		if ( deriv == 0 ) {
			int end = i + 1;
			for ( int j = i + 2; j < ( signed int )stream.size() && stream[j] == stream[i]; j++ ) {
				end++;
			}
			zeros.push_back( i + 0.5 * ( ( double )end - ( double )i ) );
			
			// Push the index over the flat extrema
			i = end - 1;
		
		// Capture other extrema
		} else if ( deriv * nextDeriv < 0 ) {
			/* Fit a parabola to the three points
			       See http://stackoverflow.com/questions/717762
			       y = Ax^2 + Bx + C
			   For simplicity of calculation, assume that the points are:
			       ( -1, stream[i] )
				   (  0, stream[i + 1] )
				   (  1, stream[i + 2] )
			*/
			//// The denominator of A and B cancel out when finding the x coordinate
			//float A = 1.0 * ( stream[i + 1] - stream[i] )
			//		- 1.0 * ( stream[i + 2] - stream[i + 1] );
			//float B = 1.0 * ( stream[i] - stream[i + 1] )
			//		+ 1.0 * ( stream[i + 1] - stream[i + 2] );
			//// The C term does not matter when finding the x coordinate
			//float vertex = -B / ( 2.0 * A );
			
			double vertex = 0.5 * ( stream[i + 2] - stream[i] )
					/ ( 2.0 * stream[i + 1] - stream[i] - stream[i + 2] );
			zeros.push_back( 1 + i + vertex );
		}
		
		// Capture zero crossings
		if ( stream[i] * stream[i + 1] < 0 ) {
			double crossing = i + stream[i] / ( double )( stream[i] - stream[i + 1] );
			
			// The crossing may lie before a point that was just added
			if ( zeros.size() > 0 && crossing < zeros.back() ) {
				double temp = zeros.back();
				zeros.pop_back();
				zeros.push_back( crossing );
				zeros.push_back( temp );
			} else {
				zeros.push_back( crossing );
			}
		}
	}
	
	// Boundary case for the final zero crossing
	if ( stream.size() > 2 ) {
		int i = stream.size() - 2;
		if ( stream[i] * stream[i + 1] < 0 ) {
			double crossing = i + stream[i] / ( double )( stream[i] - stream[i + 1] );
			zeros.push_back( crossing );
		}
	}
}