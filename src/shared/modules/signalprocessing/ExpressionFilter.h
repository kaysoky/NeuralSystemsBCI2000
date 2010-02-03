////////////////////////////////////////////////////////////////////////////////
//  $Id$
//  Author:      juergen.mellinger@uni-tuebingen.de
//  Description: A filter that uses arithmetic expressions to compute its
//    output.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef EXPRESSION_FILTER_H
#define EXPRESSION_FILTER_H

#include "GenericFilter.h"
#include "Expression/Expression.h"
#include <vector>

class ExpressionFilter : public GenericFilter
{
 public:
   ExpressionFilter();
   ~ExpressionFilter();

   void Preflight( const SignalProperties&, SignalProperties& ) const;
   void Initialize( const SignalProperties&, const SignalProperties& );
   void Process( const GenericSignal&, GenericSignal& );

 private:
   std::vector<std::vector<Expression> >  mExpressions;
};
#endif // EXPRESSION_FILTER_H
