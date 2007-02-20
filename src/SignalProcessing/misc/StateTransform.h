////////////////////////////////////////////////////////////////////////////////
// $Id$
// File:        StateTransform.h
// Date:        Jan 13, 2006
// Author:      juergen.mellinger@uni-tuebingen.de
// Description: A filter that transforms state values according to rules.
//              Whenever a given state's value changes, it replaces the new
//              value by a user-defined expression.
// $Log$
// Revision 1.1  2006/01/17 18:16:05  mellinger
// Initial version.
//
// (C) 2000-2007, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef StateTransformH
#define StateTransformH

#include "UGenericFilter.h"
#include "Expression/Expression.h"

class StateTransform : public GenericFilter
{
 public:
   StateTransform();
   ~StateTransform();

   void Preflight( const SignalProperties&, SignalProperties& ) const;
   void Initialize2( const SignalProperties&, const SignalProperties& );
   void Process( const GenericSignal*, GenericSignal* );

 private:
   std::vector<std::string> mStateNames;
   std::vector<Expression>  mExpressions;
   std::vector<int>         mPreviousInputStateValues,
                            mPreviousOutputStateValues;
};
#endif // StateTransformH
