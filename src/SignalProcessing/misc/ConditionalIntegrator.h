////////////////////////////////////////////////////////////////////////////////
// $Id$
// File:        ConditionalIntegrator.h
// Date:        Jan 2, 2006
// Author:      juergen.mellinger@uni-tuebingen.de
// Description: A filter that integrates its input signal while a given
//              expression evaluates to true.
//              This filter is intended for offline simulations of application
//              module behavior.
// $Log$
// Revision 1.2  2006/01/17 17:39:44  mellinger
// Fixed list of project files.
//
// Revision 1.1  2006/01/13 15:04:46  mellinger
// Initial version.
//
// (C) 2000-2007, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef ConditionalIntegratorH
#define ConditionalIntegratorH

#include "UGenericFilter.h"
#include "Expression/Expression.h"

class ConditionalIntegrator : public GenericFilter
{
 public:
   ConditionalIntegrator();
   ~ConditionalIntegrator();

   void Preflight( const SignalProperties&, SignalProperties& ) const;
   void Initialize2( const SignalProperties&, const SignalProperties& );
   void Process( const GenericSignal*, GenericSignal* );

 private:
   GenericSignal mSignal;
   Expression    mCondition;
   bool          mPreviousConditionValue;
};
#endif // ConditionalIntegratorH
