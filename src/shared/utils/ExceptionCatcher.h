//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class that simplifies high-level exception catching.
//   Call its Execute() function with a functor as argument in order
//   to execute the functor in a try block, catching exceptions that
//   occur during execution of the functor.
//
//   struct
//   {
//     int arg;
//     int result;
//     void operator()()
//     { result = SomeFunction( arg ); }
//   } functor = { arg, -1 };
//   ExceptionCatcher().SetMessage( "aborting" )
//                      Execute( functor );
//
// $BEGIN_BCI2000_LICENSE$
//
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2011: BCI2000 team and many external contributors ]
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
///////////////////////////////////////////////////////////////////////
#ifndef EXCEPTION_CATCHER_H
#define EXCEPTION_CATCHER_H

#include <string>

class ExceptionCatcher
{
 public:
  // Call the Execute function with a nullary functor as an argument.
  // Returns true when execution finished normally, false when an exception was caught.
  template <typename T> bool Execute( T& );
  // The Message property is a string that is appended when reporting an error.
  ExceptionCatcher&  SetMessage( const std::string& inMessage )
                     { mMessage = inMessage; return *this; }
  const std::string& Message() const
                     { return mMessage; }

 private:
  struct Fn
  {
    virtual ~Fn() {}
    virtual void Execute() = 0;
  };
  bool DoExecute1( Fn& );
  bool DoExecute2( Fn& );
#if _MSC_VER
  void ReportWin32Exception( int code );
#endif // _MSC_VER

 private:
  std::string mMessage;
};

template <typename T>
bool
ExceptionCatcher::Execute( T& inFunctor )
{
  struct : Fn
  {
    T* pFunctor;
    virtual void Execute()
    { ( *pFunctor )(); }
  } fn;
  fn.pFunctor = &inFunctor;
  return DoExecute1( fn );
}

#endif // EXCEPTION_CATCHER_H
