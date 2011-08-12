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
  string visid = FormatID( inVisID );
  new VisDisplayMemo( visid );
}

void
VisDisplay::CreateGraph( const char* inVisID )
{
  string visid = FormatID( inVisID );
  new VisDisplayGraph( visid );
}

void
VisDisplay::CreateBitmap( const char* inVisID )
{
  string visid = FormatID( inVisID );
  new VisDisplayBitmap( visid );
}

void
VisDisplay::HandleSignal( const char* inVisID, const GenericSignal& inSignal )
{
  string visid = FormatID( inVisID );
  VisDisplayBase::HandleSignal( visid.c_str(), inSignal );
}

void
VisDisplay::HandleMemo( const char* inVisID, const char* inText )
{
  string visid = FormatID( inVisID );
  VisDisplayBase::HandleMemo( visid.c_str(), inText );
}

void
VisDisplay::HandleBitmap( const char* inVisID, const BitmapImage& inBitmap )
{
  string visid = FormatID( inVisID );
  VisDisplayBase::HandleBitmap( visid.c_str(), inBitmap );
}

void
VisDisplay::HandlePropertyMessage( const char* inVisID, CfgID inCfgID, const char* inValue )
{
  string visid = FormatID( inVisID );
  string layer = Layer( visid );
  if( layer == "" )
    VisDisplayBase::HandleProperty( Base( visid ).c_str(), inCfgID, inValue, VisDisplayBase::MessageDefined );
  VisDisplayBase::HandleProperty( visid.c_str(), inCfgID, inValue, VisDisplayBase::MessageDefined );
}

void
VisDisplay::HandleProperty( const char* inVisID, CfgID inCfgID, const char* inValue )
{
  string visid = FormatID( inVisID );
  string layer = Layer( visid );
  if( layer == "" )
    VisDisplayBase::HandleProperty( Base( visid ).c_str(), inCfgID, inValue, VisDisplayBase::UserDefined );
  VisDisplayBase::HandleProperty( visid.c_str(), inCfgID, inValue, VisDisplayBase::UserDefined );
}

/*
string
VisDisplay::FormatID( const char* id )
{
  string ret( id );
  return FormatID( ret );
}
*/

string
VisDisplay::FormatID( const string &id )
{
  string ret = id;
  if( ret.find( ":" ) == string::npos )
    ret.append( ":" );
  return ret;
}

string
VisDisplay::Layer( const string &id )
{
  string ret = "";
  size_t idx = id.find( ":" );
  if( idx == string::npos )
    return ret;
  ret = id.substr( idx ).substr( 1 );
  return ret;
}

string
VisDisplay::Base( const string &id )
{
  return id.substr( 0, id.find( ":" ) );
}
