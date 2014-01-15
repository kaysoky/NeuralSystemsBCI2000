#ifndef PAIR_CHANNEL_DIFF
#define PAIR_CHANNEL_DIFF

#include "GenericFilter.h"

class PairedChannelDiffFilter : public GenericFilter {
	public:
		PairedChannelDiffFilter();
		~PairedChannelDiffFilter();

		void Preflight( const SignalProperties&, SignalProperties& ) const;
		void Initialize( const SignalProperties&, const SignalProperties& );
		void Process( const GenericSignal&, GenericSignal& );
};
#endif // PAIR_CHANNEL_DIFF
