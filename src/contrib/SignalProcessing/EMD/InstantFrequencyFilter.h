#ifndef INST_FREQ_FILTER_H
#define INST_FREQ_FILTER_H

#include "GenericFilter.h"

class InstantFrequencyFilter : public GenericFilter {
	public:
		InstantFrequencyFilter();
		~InstantFrequencyFilter();

		void Preflight( const SignalProperties&, SignalProperties& ) const;
		void Initialize( const SignalProperties&, const SignalProperties& );
		void Process( const GenericSignal&, GenericSignal& );

	private:
		int numChannels;
		std::vector<double> validFrequencies;
		double frequencyBandwidth;

		void RefinedGeneralizedZeroCrossing( const std::vector<double>&, std::list<double>& );
};
#endif // INST_FREQ_FILTER_H
