////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author:      juergen.mellinger@uni-tuebingen.de
// Description: A filter that integrates its input signal while a given
//              expression evaluates to true.
//              This filter is intended for offline simulations of application
//              module behavior.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef CONDITIONAL_INTEGRATOR_H
#define CONDITIONAL_INTEGRATOR_H

#include "GenericFilter.h"
#include "Expression/Expression.h"

class ConditionalIntegrator : public GenericFilter
{
 public:
   ConditionalIntegrator();
   ~ConditionalIntegrator();

   void Preflight( const SignalProperties&, SignalProperties& ) const;
   void Initialize( const SignalProperties&, const SignalProperties& );
   void Process( const GenericSignal&, GenericSignal& );

 private:
   GenericSignal mSignal;
   Expression    mCondition;
   bool          mPreviousConditionValue;
};
#endif // CONDITIONAL_INTEGRATOR_H
