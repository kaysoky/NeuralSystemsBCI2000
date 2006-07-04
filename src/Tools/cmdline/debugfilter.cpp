#include "PCHIncludes.h"
#pragma hdrstop

#include "UGenericFilter.h"

#define REPORTFUNCTION  { ReportProcessCount(); \
                          bciout << "entered function " __FUNC__ << endl; }

using namespace std;

class DebugFilter : public GenericFilter
{
 public:
          DebugFilter();
  virtual ~DebugFilter();
  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize();
  virtual void StartRun();
  virtual void StopRun();
  virtual void Process( const GenericSignal *Input, GenericSignal *Output );
  virtual void Resting();
  virtual void Halt();

 private:
  void ReportProcessCount() const;
  mutable unsigned long mProcessCount;
};

RegisterFilter( DebugFilter, 2.Y );

DebugFilter::DebugFilter()
: mProcessCount( 0 )
{
  REPORTFUNCTION;
}

DebugFilter::~DebugFilter()
{
  REPORTFUNCTION;
}

void DebugFilter::Preflight( const SignalProperties& inSignalProperties,
                                   SignalProperties& outSignalProperties ) const
{
  bciout << "Input Signal Properties are " << inSignalProperties << endl;
  //bcierr << "Reporting a bogus error" << endl;
  outSignalProperties = inSignalProperties;
}

void DebugFilter::Initialize()
{
  REPORTFUNCTION;
}

void DebugFilter::StartRun()
{
  REPORTFUNCTION;
}

void DebugFilter::StopRun()
{
  REPORTFUNCTION;
}

void DebugFilter::Process( const GenericSignal *input, GenericSignal *output )
{
  ++mProcessCount;
  *output = *input;
}

void DebugFilter::Resting()
{
  REPORTFUNCTION;
}

void DebugFilter::Halt()
{
  REPORTFUNCTION;
}

void DebugFilter::ReportProcessCount() const
{
  if( mProcessCount > 0 )
  {
    __bciout << "DebugFilter::Process called "
             << mProcessCount
             << " times"
             << endl;
  }
  mProcessCount = 0;
}
