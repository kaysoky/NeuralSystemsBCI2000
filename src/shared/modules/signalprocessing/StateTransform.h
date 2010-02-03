////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author:      juergen.mellinger@uni-tuebingen.de
// Description: A filter that transforms state values according to rules.
//              Whenever a given state's value changes, it replaces the new
//              value by a user-defined expression.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef STATE_TRANSFORM_H
#define STATE_TRANSFORM_H

#include "GenericFilter.h"
#include "Expression/Expression.h"

class StateTransform : public GenericFilter
{
 public:
   StateTransform();
   ~StateTransform();

   void Preflight( const SignalProperties&, SignalProperties& ) const;
   void Initialize( const SignalProperties&, const SignalProperties& );
   void Process( const GenericSignal&, GenericSignal& );

 private:
   std::vector<std::string> mStateNames;
   std::vector<Expression>  mExpressions;
   std::vector<int>         mPreviousInputStateValues,
                            mPreviousOutputStateValues;
};
#endif // STATE_TRANSFORM_H
