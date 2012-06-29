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
///////////////////////////////////////////////////////////////////////
#ifndef BCI_EXCEPTION_H
#define BCI_EXCEPTION_H

#include <exception>
#include <sstream>

#define STR_(x) STR__(x)
#define STR__(x) #x

#ifndef __BORLANDC__
# if defined( __FUNCTION__ )
#  define FUNCTION_ __FUNCTION__
# elif defined( __PRETTY_FUNCTION__ )
#  define FUNCTION_   __PRETTY_FUNCTION__
# elif defined( __func__ )
#  define FUNCTION_   __func__
# endif
#endif // __BORLANDC__

#define FILE_CONTEXT_ \
     "\nFile: " << __FILE__ \
  << "\nLine: " << STR_( __LINE__ )

#ifdef FUNCTION_
# define EXCEPTION_CONTEXT_ \
     "\nFunction: " << FUNCTION_ << "()" \
  << FILE_CONTEXT_
#else
# define EXCEPTION_CONTEXT_  FILE_CONTEXT_
#endif

// Due to the use of a temporary, we need to use an inserter that's implemented as an ostream member first.
// Especially, std::operator<<( ostream&, const char* ) cannot be used as an initial inserter because its
// first argument (reference) cannot be initialized with a temporary. The expression would be resolved to
// std::ostream::operator<<( void* ), and the address of the string literal inserted into the stream
// rather than the string itself.
#define bciexception_(x) BCIException( std::ostringstream() << std::flush << x )
#define bciexception(x)  bciexception_( x << EXCEPTION_CONTEXT_ )

class BCIException : public std::exception
{
 public:
  BCIException( const std::string& inMessage )
    : mMessage( inMessage )
    {}
  BCIException( std::ostream& );

  virtual ~BCIException() throw()
    {}

  virtual const char* what() const throw();

 private:
  std::string mMessage;
};

#endif // BCI_EXCEPTION_H

