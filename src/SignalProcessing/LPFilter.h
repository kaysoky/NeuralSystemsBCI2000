////////////////////////////////////////////////////////////////////////////////
//
//  File:        LPFilter.h
//
//  Description: A (working) tutorial low pass filter demonstrating
//               parameter access, visualization, and unit conversion.
//
//  Date:        May 27, 2005
//  Author:      juergen.mellinger@uni-tuebingen.de
//
////////////////////////////////////////////////////////////////////////////////
#ifndef LPFilterH
#define LPFilterH

#include "UGenericFilter.h"
#include "UGenericVisualization.h"

class LPFilter : public GenericFilter
{
 public:
   LPFilter();
   ~LPFilter();

   void Preflight( const SignalProperties&, SignalProperties& ) const;
   void Initialize();
   void Process( const GenericSignal*, GenericSignal* );

 private:
   float                mTimeConstant,
                        mDecayFactor;
   std::vector<float>   mPreviousOutput;
   
   GenericVisualization mSignalVis;
};
#endif // LPFilterH
