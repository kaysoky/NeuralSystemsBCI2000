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
// (C) 2000-2007, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef SetBaselineH
#define SetBaselineH

#include "UGenericFilter.h"
#include "UGenericVisualization.h"
#include <vector>

class TSetBaseline : public GenericFilter
{
  public:
                   TSetBaseline();
    virtual        ~TSetBaseline() {}

    virtual void   Preflight( const SignalProperties&, SignalProperties& ) const;
    virtual void   Initialize();
    virtual void   Process( const GenericSignal* InputSignal, GenericSignal* );

  private:
    int                  mVisualize;
    GenericVisualization mVis;

    // BL variables.
    int                  mLastBaselineState;
    std::vector<float>   mBLSamples;
    std::vector<bool>    mBaseChList;
    GenericSignal        mBLSignal;
};

#endif // SetBaselineH
