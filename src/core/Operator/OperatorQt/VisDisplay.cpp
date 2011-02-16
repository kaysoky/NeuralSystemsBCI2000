////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: An interface class for visualization displays.
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
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "VisDisplay.h"

#include "VisDisplayBase.h"
#include "VisDisplayMemo.h"
#include "VisDisplayGraph.h"
#include "VisDisplayBitmap.h"

using namespace std;

void
VisDisplay::SetParentWindow( QWidget* inW )
{
  VisDisplayBase::SetParentWindow( inW );
}

void
VisDisplay::Clear()
{
  VisDisplayBase::Clear();
}

void
VisDisplay::CreateMemoWindow( const char* inVisID )
{
  new VisDisplayMemo( inVisID );
}

void
VisDisplay::CreateGraphWindow( const char* inVisID )
{
  new VisDisplayGraph( inVisID );
}

void
VisDisplay::CreateBitmapWindow( const char* inVisID )
{
  new VisDisplayBitmap( inVisID );
}

void
VisDisplay::HandleSignal( const char* inVisID, const GenericSignal& inSignal )
{
  VisDisplayBase::HandleSignal( inVisID, inSignal );
}

void
VisDisplay::HandleMemo( const char* inVisID, const char* inText )
{
  VisDisplayBase::HandleMemo( inVisID, inText );
}

void
VisDisplay::HandleBitmap( const char* inVisID, const BitmapImage& inBitmap )
{
  VisDisplayBase::HandleBitmap( inVisID, inBitmap );
}

void
VisDisplay::HandlePropertyMessage( const char* inVisID, const IDType inCfgID, const char* inValue )
{
  VisDisplayBase::HandleProperty( inVisID, inCfgID, inValue, VisDisplayBase::MessageDefined );
}

void
VisDisplay::HandleProperty( const char* inVisID, const IDType inCfgID, const char* inValue )
{
  VisDisplayBase::HandleProperty( inVisID, inCfgID, inValue, VisDisplayBase::UserDefined );
}
