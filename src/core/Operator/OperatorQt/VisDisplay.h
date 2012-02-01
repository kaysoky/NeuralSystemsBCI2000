////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: An interface class for visualization displays.
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
#ifndef VIS_DISPLAY_H
#define VIS_DISPLAY_H

#include "CfgID.h"
#include <string>

class GenericSignal;
class BitmapImage;
class QWidget;

class VisDisplay
{
 public:
  static void SetParentWindow( QWidget* );
  static void CreateMemo( const char* visID );
  static void CreateGraph( const char* visID );
  static void CreateBitmap( const char* visID );
  static void HandleSignal( const char* visID, const GenericSignal& );
  static void HandleMemo( const char* visID, const char* );
  static void HandleBitmap( const char* visID, const BitmapImage& );
  static void HandlePropertyMessage( const char* visID, CfgID, const char* );
  static void HandleProperty( const char* visID, CfgID, const char* );
  static void Clear();
};

#endif // VIS_DISPLAY_H
