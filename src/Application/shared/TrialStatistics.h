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

    int   Hits()  const;
    int   Total() const;
    float Bits()  const;

  private:
    std::vector<std::vector<int> > mTargetsResultsMatrix;
};

#endif // TrialStatisticsH
