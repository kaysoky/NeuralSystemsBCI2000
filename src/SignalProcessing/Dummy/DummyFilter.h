/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
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




