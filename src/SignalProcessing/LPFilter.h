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
// (C) 2000-2007, BCI2000 Project
// http://www.bci2000.org
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
   void Initialize2( const SignalProperties&, const SignalProperties& );
   void Process( const GenericSignal*, GenericSignal* );

 private:
   
   float                mDecayFactor;
   std::vector<float>   mPreviousOutput;

   GenericVisualization mSignalVis;
};
#endif // LPFilterH
