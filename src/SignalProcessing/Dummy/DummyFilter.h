#ifndef DummyFilterH
#define DummyFilterH

#include "UGenericFilter.h"

class DummyFilter : public GenericFilter
{
 public:
          DummyFilter();
  virtual ~DummyFilter();
  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize();
  virtual void Process(const GenericSignal *Input, GenericSignal *Output);
};

#endif // DummyFilterH


