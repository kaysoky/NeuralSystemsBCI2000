#include "PCHIncludes.h" // Make the compiler's Pre-Compiled Headers feature happy
#pragma hdrstop

#include "PairedChannelDiffFilter.h"

#include "MeasurementUnits.h"
#include "BCIError.h"

using namespace std;

RegisterFilter( PairedChannelDiffFilter, 2.A1 );

PairedChannelDiffFilter::PairedChannelDiffFilter() { }

PairedChannelDiffFilter::~PairedChannelDiffFilter() { }

void PairedChannelDiffFilter::Preflight( const SignalProperties& Input,
                                         SignalProperties& Output ) const {
	// Must have an even number of channels
    PreflightCondition( Input.Channels() % 2 == 0 );
	Output = Input;
	Output.SetChannels( Input.Channels() / 2 );

	// Keep the channel label for the even-indexed input channel
	for ( int channel = 0; channel < Output.Channels(); channel++ ) {
		Output.ChannelLabels()[channel] = Input.ChannelLabels()[channel * 2];
	}
}

void PairedChannelDiffFilter::Initialize( const SignalProperties& Input,
                                          const SignalProperties& Output ) { }

void PairedChannelDiffFilter::Process( const GenericSignal& Input, GenericSignal& Output ) {
    // Subtract from each even-indexed input channel its following input channel
	for ( int channel = 0; channel < Output.Channels(); channel++ ) {
		for ( int sample = 0; sample < Output.Elements(); sample++ ) {
			Output( channel, sample ) = Input( 2 * channel, sample ) - Input( 2 * channel + 1, sample );
		}
	}
}