//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: An exception class to report BCI2000 run-time errors,
//   and a std::ostream based object that allows convenient creation
//   of exceptions with message content.
//   To throw a BCI2000 exception, write:
//
//   #include "BCIException.h"
//   ...
//   throw bciexception( "Illegal value of n: " << n );
//
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
#ifndef BCI_EXCEPTION_H
#define BCI_EXCEPTION_H

#include <exception>
#include <sstream>

#ifdef _MSC_VER
# define EXCEPTION_CONTEXT_   __FUNCTION__
#else
# define EXCEPTION_CONTEXT_   __FUNC__
#endif // _MSC_VER

#define bciexception(x) BCIException( std::ostringstream() << EXCEPTION_CONTEXT_ << ": " << x )

class BCIException : public std::exception
{
 public:
  BCIException( const std::string& inMessage )
    : mMessage( inMessage )
    {}
  BCIException( std::ostream& );

  virtual const char* what() const throw();

 private:
  std::string mMessage;
};

#endif // BCI_EXCEPTION_H

