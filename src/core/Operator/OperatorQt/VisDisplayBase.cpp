////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: A base class for visualization displays.
//   Also handles message dispatching and storage of visualization properties.
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

#include "VisDisplayBase.h"
#include "VisDisplayWindow.h"
#include "VisDisplayGraph.h"
#include "VisDisplayMemo.h"
#include "VisDisplayBitmap.h"
#include "Settings.h"
#include "defines.h"

#include <QtGui>

using namespace std;

VisDisplayBase::VisContainer&
VisDisplayBase::Visuals()
{
  static VisDisplayBase::VisContainer visuals;
  return visuals;
}

VisDisplayBase::ConfigContainer&
VisDisplayBase::Visconfigs()
{
  static VisDisplayBase::ConfigContainer visconfigs;
  return visconfigs;
}

////////////////////////////////////////////////////////////////////////////////
static const QString cfgid_prefix = "CFGID";

void
VisDisplayBase::ConfigContainer::Save()
{
  Settings settings;
  settings.beginGroup( KEY_VISUALIZATION );
  for( iterator i = begin(); i != end(); ++i )
  {
    settings.beginGroup( i->first.c_str() );
    settings.setValue( "Title", i->second[ CfgID::WindowTitle ].c_str() );
    for( ConfigSettings::iterator j = i->second.begin(); j != i->second.end(); ++j )
    {
      if( i->second.State( j->first ) == UserDefined )
      {
        QString cfgID;
        cfgID.setNum( j->first );
        settings.setValue( cfgid_prefix + cfgID, j->second.c_str() );
      }
    }
    settings.endGroup();
  }
}

void
VisDisplayBase::ConfigContainer::Restore()
{
  Settings settings;
  settings.beginGroup( KEY_VISUALIZATION );
  QStringList visIDs = settings.childGroups();
  for( QStringList::iterator i = visIDs.begin(); i != visIDs.end(); ++i )
  {
    string visID = i->toLocal8Bit().constData();
    settings.beginGroup( *i );
    QStringList cfgIDs = settings.childKeys();
    for( QStringList::iterator j = cfgIDs.begin(); j != cfgIDs.end(); ++j )
    {
      if( j->startsWith( cfgid_prefix ) )
      {
        CfgID cfgID = j->mid( cfgid_prefix.length() ).toUInt();
        QString value = settings.value( *j ).toString();
        ( *this )[ visID ].Put( cfgID, value.toStdString(), OnceUserDefined );
      }
    }
    settings.endGroup();
  }
}

////////////////////////////////////////////////////////////////////////////////
VisDisplayBase::VisDisplayBase( const std::string& inVisID )
: QWidget( NULL ),
  mVisID( inVisID )
{
  if( mVisID != mVisID.WindowID() )
  {
    if( !Visuals()[ mVisID.WindowID() ] )
      new VisDisplayWindow( mVisID.WindowID() );
    this->setParent( Visuals()[ mVisID.WindowID() ] );
  }
  VisDisplayBase* visual = Visuals()[ mVisID ];
  delete visual;
  Visuals()[ mVisID ] = this;
}

VisDisplayBase::~VisDisplayBase()
{
  Visuals()[ mVisID ] = NULL;
}

void
VisDisplayBase::VisContainer::Clear()
{
  for( iterator i = begin(); i != end(); ++i )
    delete i->second;
  VisContainerBase::clear();
}

void
VisDisplayBase::SetConfig( ConfigSettings& inConfig )
{
  bool visible = true;
  inConfig.Get( CfgID::Visible, visible );
  this->setVisible( visible );
}

void
VisDisplayBase::Restore()
{
  SetConfig( Visconfigs()[ mVisID ] );
}

void
VisDisplayBase::Save() const
{
}

void
VisDisplayBase::HandleSignal( const char* inVisID, const GenericSignal& inSignal )
{
  VisDisplayGraph* visual = dynamic_cast<VisDisplayGraph*>( Visuals()[ inVisID ] );
  if( visual != NULL )
    QMetaObject::invokeMethod(
        visual,
        "HandleSignal",
        Qt::QueuedConnection,
        Q_ARG( GenericSignal, inSignal ) );
}

void
VisDisplayBase::HandleMemo( const char* inVisID, const char* inText )
{
  VisDisplayMemo* visual = dynamic_cast<VisDisplayMemo*>( Visuals()[ inVisID ] );
  if( visual != NULL )
    QMetaObject::invokeMethod(
        visual,
        "HandleMemo",
        Qt::QueuedConnection,
        Q_ARG( QString, inText ) );
}

void
VisDisplayBase::HandleBitmap( const char* inVisID, const BitmapImage& inBitmap )
{
  VisDisplayBitmap* visual = dynamic_cast<VisDisplayBitmap*>( Visuals()[ inVisID ] );
  if( visual != NULL )
    QMetaObject::invokeMethod(
        visual,
        "HandleBitmap",
        Qt::QueuedConnection,
        Q_ARG( BitmapImage, inBitmap ) );
}

void
VisDisplayBase::HandleProperty( const char* inVisID, CfgID inCfgID, const char* inValue, ConfigState inState )
{
  Visconfigs()[ inVisID ].Put( inCfgID, inValue, inState );
  if( Visuals()[ inVisID ] != NULL )
    Visuals()[ inVisID ]->SetConfig( Visconfigs()[ inVisID ] );
}

