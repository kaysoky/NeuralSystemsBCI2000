////////////////////////////////////////////////////////////////////////////////
//  $Id$
//  File:        RandomFilter.h
//  Date:        Oct 13, 2005
//  Author:      juergen.mellinger@uni-tuebingen.de
//  Description: A filter that returns zero-mean white noise multiplied by the
//               input signal's value.
//  $Log$
//  Revision 1.1  2006/01/13 15:04:46  mellinger
//  Initial version.
//
// (C) 2000-2007, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef RandomFilterH
#define RandomFilterH

#include "UGenericFilter.h"

class RandomFilter : public GenericFilter
{
 public:
   RandomFilter();
   ~RandomFilter();

   void Preflight( const SignalProperties&, SignalProperties& ) const;
   void Initialize2( const SignalProperties&, const SignalProperties& );
   void Process( const GenericSignal*, GenericSignal* );

 private:
   static float GetRandomUniform();
   float ( *mpGetRandom )();
   class GenericVisualization* mpVis;
   enum
   {
     none,
     uniform,
   } RandomType;
};
#endif // RandomFilterH
