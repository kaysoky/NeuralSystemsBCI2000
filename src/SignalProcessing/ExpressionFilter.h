////////////////////////////////////////////////////////////////////////////////
//  $Id$
//  File:        ExpressionFilter.h
//
//  Description: A tutorial filter demonstrating
//               the use of boolean and arithmetic expressions in parameters.
//               -- Each time the WarningExpression expression evaluates to true,
//                  a warning is issued.
//               -- Filter output is determined by the expressions given in the
//                  Expressions matrix.
//
//  Date:        Aug 12, 2005
//  Author:      juergen.mellinger@uni-tuebingen.de
//  $Log$
//  Revision 1.2  2006/01/12 20:41:34  mellinger
//  Added CVS id and log to comment.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef ExpressionFilterH
#define ExpressionFilterH

#include "UGenericFilter.h"
#include "Expression/Expression.h"
#include <vector>

class ExpressionFilter : public GenericFilter
{
 public:
   ExpressionFilter();
   ~ExpressionFilter();

   void Preflight( const SignalProperties&, SignalProperties& ) const;
   void Initialize2( const SignalProperties&, const SignalProperties& );
   void Process( const GenericSignal*, GenericSignal* );

 private:
   Expression                             mWarningExpression;
   std::vector<std::vector<Expression> >  mExpressions;
};
#endif // ExpressionFilterH
