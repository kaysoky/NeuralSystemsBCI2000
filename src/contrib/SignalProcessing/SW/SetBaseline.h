////////////////////////////////////////////////////////////////////////////////
//
// File: SetBaseline.h
//
// Description: Slow Wave Class Definition
//           written by Dr. Thilo Hinterberger 2000-2001
//           Copyright University of Tuebingen, Germany
//
// Changes:  2003, juergen.mellinger@uni-tuebingen.de: some bugs fixed.
//           Feb 8, 2004, jm: Adaptations to changes in BCI2000 framework,
//           minor reformulations, reformatting.
//           Feb 24, 2004, jm: Moved the TSetBaseline class into separate files.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef SetBaselineH
#define SetBaselineH

#include "GenericFilter.h"
#include "GenericVisualization.h"
#include <vector>

class TSetBaseline : public GenericFilter
{
  public:
                   TSetBaseline();
    virtual        ~TSetBaseline() {}

    virtual void   Preflight( const SignalProperties&, SignalProperties& ) const;
    virtual void   Initialize(  const SignalProperties&, const SignalProperties&  );
    virtual void   Process( const GenericSignal& InputSignal, GenericSignal& );

  private:
    // BL variables.
    int                  mLastBaselineState;
    std::vector<float>   mBLSamples;
    std::vector<bool>    mBaseChList;
    GenericSignal        mBLSignal;
};

#endif // SetBaselineH
