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
#include "PCHIncludes.h"
#pragma hdrstop

#include "VisDisplay.h"

#include "VisID.h"
#include "VisDisplayBase.h"
#include "VisDisplayWindow.h"
#include "VisDisplayMemo.h"
#include "VisDisplayGraph.h"
#include "VisDisplayBitmap.h"

using namespace std;

void
VisDisplay::SetParentWindow( QWidget* inW )
{
  VisDisplayWindow::SetParentWindow( inW );
}

void
VisDisplay::Clear()
{
  VisDisplayBase::Clear();
}

void
VisDisplay::CreateMemo( const char* inVisID )
{
  new VisDisplayMemo( VisID( inVisID ).ToLayer() );
}

void
VisDisplay::CreateGraph( const char* inVisID )
{
  new VisDisplayGraph( VisID( inVisID ).ToLayer() );
}

void
VisDisplay::CreateBitmap( const char* inVisID )
{
  new VisDisplayBitmap( VisID( inVisID ).ToLayer() );
}

void
VisDisplay::HandleSignal( const char* inVisID, const GenericSignal& inSignal )
{
  VisDisplayBase::HandleSignal( VisID( inVisID ).ToLayer(), inSignal );
}

void
VisDisplay::HandleMemo( const char* inVisID, const char* inText )
{
  VisDisplayBase::HandleMemo( VisID( inVisID ).ToLayer(), inText );
}

void
VisDisplay::HandleBitmap( const char* inVisID, const BitmapImage& inBitmap )
{
  VisDisplayBase::HandleBitmap( VisID( inVisID ).ToLayer(), inBitmap );
}

void
VisDisplay::HandlePropertyMessage( const char* inVisID, CfgID inCfgID, const char* inValue )
{
  VisID visID( inVisID );
  if( visID.LayerID().empty() )
    VisDisplayBase::HandleProperty( visID.WindowID(), inCfgID, inValue, VisDisplayBase::MessageDefined );
  VisDisplayBase::HandleProperty( visID.ToLayer(), inCfgID, inValue, VisDisplayBase::MessageDefined );
}

void
VisDisplay::HandleProperty( const char* inVisID, CfgID inCfgID, const char* inValue )
{
  VisID visID( inVisID );
  if( visID.LayerID().empty() )
    VisDisplayBase::HandleProperty( visID.WindowID(), inCfgID, inValue, VisDisplayBase::UserDefined );
  VisDisplayBase::HandleProperty( visID.ToLayer(), inCfgID, inValue, VisDisplayBase::UserDefined );
}
