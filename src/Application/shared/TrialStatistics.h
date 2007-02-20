////////////////////////////////////////////////////////////////////////////////
//
// File:   TrialStatistics.h
//
// Date:   Feb 8, 2004
//
// Author: juergen.mellinger@uni-tuebingen.de
//
// Description: A class that encapsulates trial statistics for an application.
//
// (C) 2000-2007, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef TrialStatisticsH
#define TrialStatisticsH

#include "UEnvironment.h"

class TrialStatistics: public Environment
{
  public:
    void  Preflight() const;
    void  Initialize();

    void  Reset();
    void  Update( int inTargetCode, int inResultCode );
    void  UpdateInvalid();

    int   Hits()    const;
    int   Total()   const;
    int   Invalid() const;
    float Bits()    const;

  private:
    int                            mInvalidTrials;
    std::vector<std::vector<int> > mTargetsResultsMatrix;
};

#endif // TrialStatisticsH
