//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: Classes for convenient throwing and catching of
//   expected and unexpected exceptions.
//
//   #include "BCIException.h"
//   ...
//   throw std_invalid_argument( "Illegal value of n: " << n );
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
#include <typeinfo>
#include "Compiler.h"

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
#define EXCEPTION_ARG_(x)       bci::Exception::ToString( std::ostringstream() << std::flush << x )
#define stdexception(type,x)    bci::Exception_<std::type>( EXCEPTION_ARG_(x), EXCEPTION_ARG_( EXCEPTION_CONTEXT_ ) )
#define std_logic_error(x)      stdexception(logic_error,x)
#define std_domain_error(x)     stdexception(domain_error,x)
#define std_invalid_argument(x) stdexception(invalid_argument,x)
#define std_length_error(x)     stdexception(length_error,x)
#define std_runtime_error(x)    stdexception(runtime_error,x)
#define std_out_of_range(x)     stdexception(out_of_range,x)
#define std_range_error(x)      stdexception(range_error,x)
#define std_overflow_error(x)   stdexception(overflow_error,x)
#define std_underflow_error(x)  stdexception(underflow_error,x)
#define std_bad_alloc(x)        stdexception(bad_alloc,x)

#define bciexception(x)         stdexception(exception,x)

namespace bci {
  class Exception
  {
   public:
    explicit Exception(
      const std::string& inWhat,
      const std::string& inWhere = "",
      const std::type_info& inType = typeid( Exception )
    );
    virtual ~Exception() throw() {}

    virtual const std::string& Where() const throw() { return mWhere; }
    virtual const std::string& What() const throw() { return mWhat; }

    static std::string ToString( std::ostream& );

    private:
     std::string mWhat, mWhere;
  };

  template<typename T>
  struct Exception_ : Exception, T
  {
    explicit Exception_( const std::string& what, const std::string& where = "" )
      : T( ( what + where ).c_str() ), Exception( what, where, typeid( T ) ) {}
    virtual ~Exception_() throw() {}
  };
} // namespace bci

typedef bci::Exception BCIException;

#endif // BCI_EXCEPTION_H
