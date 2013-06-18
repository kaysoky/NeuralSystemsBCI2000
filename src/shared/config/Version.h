////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: Default Version.h file. By executing
//   build/buildutils/UpdateVersionHeader, this file will be replaced with more
//   accurate information.
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
////////////////////////////////////////////////////////////////////////////////
#ifndef VERSION_H
#define VERSION_H

#include "Compiler.h"

#ifdef PROJECT_VERSION
# define VERSION_ "$Version: " PROJECT_VERSION " $"
#else
# define VERSION_
#endif
#ifdef PROJECT_REVISION
# define SOURCE_REVISION_ "$Source Revision: " PROJECT_REVISION " $"
#else
# define SOURCE_REVISION_
#endif
#ifdef PROJECT_DATE
# define SOURCE_DATE_ "$Source Date: " PROJECT_DATE " $"
#else
# define SOURCE_DATE_
#endif
#ifdef BUILD_TYPE
# define BUILD_TYPE_ "$Build Type: " BUILD_TYPE " $"
#else
# define BUILD_TYPE_
#endif
#ifdef BUILD_USER
# define BUILD_USER_ "$Build User: " BUILD_USER " $"
#else
# define BUILD_USER_
#endif
#ifdef BUILD_CONFIG
# define BUILD_CONFIG_ "$Config: " BUILD_CONFIG " $"
#else
# define BUILD_CONFIG_
#endif

#define PROJECT_VERSION_DEF  VERSION_ SOURCE_REVISION_ SOURCE_DATE_ \
                             BUILD_TYPE_ BUILD_USER_ BUILD_CONFIG_ \
                            "$Build Date: " __DATE__ " " __TIME__ " $" \
                            "$Compiler: " COMPILER_NAME " $"

#define PROJECT_COPYRIGHT "(C) 2000-2013, " PROJECT_NAME " Project\n" \
                          "http://www." PROJECT_DOMAIN

#endif // VERSION_H
