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

#ifndef PROJECT_VERSION
# define PROJECT_VERSION "3.0"
#endif

#define PROJECT_VERSION_DEF  "$Version: " PROJECT_VERSION " $" \
                     "$Build Date: " __DATE__ " " __TIME__ " $" \
                     "$Compiler: " COMPILER_NAME " $"

#define PROJECT_COPYRIGHT "(C) 2000-2013, " PROJECT_NAME " Project\n" \
                          "http://www." PROJECT_DOMAIN

#endif // VERSION_H
