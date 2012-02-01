//////////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A collection of classes that represent nodes of parsed expressions.
//
// $BEGIN_BCI2000_LICENSE$
//
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2012: BCI2000 team and many external contributors ]
//
// BCI2000 is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// BCI2000 is distributed in the hope that it will be useful, but
//                         WITHOUT ANY WARRANTY
// - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <http://www.gnu.org/licenses/>.
//
// $END_BCI2000_LICENSE$
//////////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "ExpressionNodes.h"
#include "BCIException.h"
#include <sstream>

using namespace std;
using namespace ExpressionParser;

// ExpressionNodes::Node
Node::Node( bool isConst )
: mIsConst( isConst )
{
}

Node::~Node()
{
  for( size_t i = 0; i < mChildren.size(); ++i )
    delete mChildren[i];
}

Node*
Node::Simplify()
{
  for( size_t i = 0; i < mChildren.size(); ++i )
    mChildren[i] = mChildren[i]->Simplify();
  return OnSimplify();
}

void
Node::Add( Node* p )
{
  if( p )
    mChildren.push_back( p );
}

// ExpressionNodes::ConstPropagatingNode
Node*
ConstPropagatingNode::OnSimplify()
{
  Node* result = this;
  if( mPropagate )
  {
    bool allChildrenConst = true;
    for( size_t i = 0; i < mChildren.size(); ++i )
      allChildrenConst &= mChildren[i]->IsConst();
    if( allChildrenConst )
    {
      result = new ConstantNode( this->Evaluate() );
      delete this;
    }
  }
  return result;
}


// ExpressionNodes::AddressNode
string
AddressNode::EvaluateToString() const
{
  ostringstream oss;
  if( !mChildren.empty() )
    oss << mChildren[0]->Evaluate();
  if( mpString )
    oss << *mpString;
  return oss.str();
}

Node*
AddressNode::OnSimplify()
{
  mIsConst = ( mChildren.empty() || mChildren[0]->IsConst() );
  return this;
}

double
AddressNode::OnEvaluate()
{
  throw bciexception( "This function should never be called" );
  return 0;
}

