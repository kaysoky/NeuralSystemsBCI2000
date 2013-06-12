//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: Debugging support.
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
#include "PCHIncludes.h"
#pragma hdrstop

#include "Debugging.h"
#include "FileUtils.h"

#if _WIN32
# include "Windows.h"
#else
# include <iostream>
#endif

using namespace std;

void
bci::SuggestDebugging( const string& inReason, const string& inDetails )
{
#if BCIDEBUG
  string name = FileUtils::ExtractBase( FileUtils::ExecutablePath() ),
         title = inReason.empty() ?
                 name + " (Debug Build) suggests debugging" :
                 inReason + " in " + name + " (Debug Build)";

#if _WIN32

  string text = inDetails;
  if( !text.empty() )
    text += "\n\n";
  text += "This dialog window gives you the opportunity to attach a debugger."
          "\nIf you don't want to debug, or if you don't know what this is all about, press OK to continue.";
  ::MessageBoxA( 0, text.c_str(), title.c_str(), MB_OK );

#else  // _WIN32

  cout << title << "\n"
       << "Execution is now halted, so you may attach a debugger to the process.\n"
       << "If you don't want to debug, or if you don't know what this is all about, press Enter to continue.\n";
  getline( name, cin.flush() );

#endif // _WIN32

#endif // BCIDEBUG
}
