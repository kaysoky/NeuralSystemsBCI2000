/////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class for OS error codes and messages.
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
/////////////////////////////////////////////////////////////////////////////
#ifndef OS_ERROR_H
#define OS_ERROR_H

#include <string>
#include "Uncopyable.h"

class OSError : private Uncopyable
{
 public:
  OSError(); // This constructor obtains the last error code.
  OSError( long inErrorCode )
  : mCode( inErrorCode ),
    mMessage( cDefaultMessage )
  {}
  // Properties
  long        Code() const
              { return mCode; }
  const char* Message() const;

 private:
  long mCode;
  mutable std::string mMessage;

  static const std::string cDefaultMessage;
};

#endif // OS_ERROR_H
