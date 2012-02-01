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
#ifndef EXPRESSION_NODES_H
#define EXPRESSION_NODES_H

#include <vector>
#include <string>

namespace ExpressionParser
{

typedef std::vector<class Node*> NodeList;

class Node
{
 public:
  Node( bool isConst = false );
  virtual ~Node();

  bool IsConst() const { return mIsConst; }

  Node* Simplify();
  double Evaluate() { return OnEvaluate(); }

 protected:
  void Add( Node* );

 protected:
  virtual Node* OnSimplify() { return this; }
  virtual double OnEvaluate() = 0;

 protected:
  NodeList mChildren;
  bool mIsConst;
};

class ConstantNode : public Node
{
 public:
  ConstantNode( double value ) : Node( true ), mValue( value ) {}

 protected:
  double OnEvaluate() { return mValue; }

 private:
  double mValue;
};

class VariableNode : public Node
{
 public:
  VariableNode( double& valueRef ) : mrValue( valueRef ) {}

 protected:
  double OnEvaluate() { return mrValue; }

 private:
  double& mrValue;
};

class AssignmentNode : public Node
{
 public:
  AssignmentNode( double& valueRef, Node* rhs ) : mrValue( valueRef ) { Add( rhs ); }

 protected:
  double OnEvaluate() { return ( mrValue = mChildren[0]->Evaluate() ); }

private:
 double& mrValue;
};

class ConstPropagatingNode : public Node
{
 public:
  ConstPropagatingNode( bool doPropagate = false ) : mPropagate( doPropagate ) {}
  Node* OnSimplify();

 private:
  bool mPropagate;
};

template<int N>
class FunctionNode {};

template<>
class FunctionNode<0> : public ConstPropagatingNode
{
 public:
  typedef double ( *Pointer )();
  FunctionNode( bool c, Pointer f ) : ConstPropagatingNode( c ), p( f ) {}

 protected:
  double OnEvaluate() { return p(); }

 private:
  Pointer p;
};

template<>
class FunctionNode<1> : public ConstPropagatingNode
{
 public:
  typedef double ( *Pointer )( double );
  FunctionNode( bool c, Pointer f, Node* arg1 ) : ConstPropagatingNode( c ), p( f ) { Add( arg1 ); }

 protected:
  double OnEvaluate() { return p( mChildren[0]->Evaluate() ); }

 private:
  Pointer p;
};

template<>
class FunctionNode<2> : public ConstPropagatingNode
{
 public:
  typedef double ( *Pointer )( double, double );
  FunctionNode( bool c, Pointer f, Node* arg1, Node* arg2 ) : ConstPropagatingNode( c ), p( f ) { Add( arg1 ); Add( arg2 ); }

 protected:
  double OnEvaluate() { return p( mChildren[0]->Evaluate(), mChildren[1]->Evaluate() ); }

 private:
  Pointer p;
};

template<>
class FunctionNode<3> : public ConstPropagatingNode
{
 public:
  typedef double ( *Pointer )( double, double, double );
  FunctionNode( bool c, Pointer f, Node* arg1, Node* arg2, Node* arg3 ) : ConstPropagatingNode( c ), p( f ) { Add( arg1 ); Add( arg2 ); Add( arg3 ); }

 protected:
  double OnEvaluate() { return p( mChildren[0]->Evaluate(), mChildren[1]->Evaluate(), mChildren[2]->Evaluate() ); }

 private:
  Pointer p;
};

class AddressNode : public Node
{
 public:
  AddressNode( Node* inNode, const std::string* inString ) : mpString( inString ) { Add( inNode ); }
  std::string EvaluateToString() const;

 protected:
  Node* OnSimplify();
  double OnEvaluate();

 private:
  const std::string* mpString;
};

} // namespace ExpressionNodes

#endif // EXPRESSION_NODES_H

