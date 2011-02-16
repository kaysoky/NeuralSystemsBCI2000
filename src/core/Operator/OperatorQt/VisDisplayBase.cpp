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
#include "VisDisplayGraph.h"
#include "VisDisplayMemo.h"
#include "VisDisplayBitmap.h"
#include "defines.h"

#include <QSettings>
#include <QtGui>

using namespace std;

QWidget* VisDisplayBase::spParentWindow = NULL;

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
  QSettings settings;
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
  QSettings settings;
  settings.beginGroup( KEY_VISUALIZATION );
  QStringList visIDs = settings.childGroups();
  for( QStringList::iterator i = visIDs.begin(); i != visIDs.end(); ++i )
  {
    string visID = i->toStdString();
    settings.beginGroup( *i );
    QStringList cfgIDs = settings.childKeys();
    for( QStringList::iterator j = cfgIDs.begin(); j != cfgIDs.end(); ++j )
    {
      if( j->startsWith( cfgid_prefix ) )
      {
        VisDisplay::IDType cfgID = j->mid( cfgid_prefix.length() ).toUInt();
        QString value = settings.value( *j ).toString();
        ( *this )[ visID ].Put( cfgID, value.toStdString(), OnceUserDefined );
      }
    }
    settings.endGroup();
  }
}

////////////////////////////////////////////////////////////////////////////////
VisDisplayBase::VisDisplayBase( const std::string& inSourceID )
: QWidget( spParentWindow ),
  mSourceID( inSourceID ),
  mUserIsMoving( true )
{
  VisDisplayBase* visual = Visuals()[ mSourceID ];
  delete visual;
  Visuals()[ mSourceID ] = this;

  this->setWindowFlags( Qt::Tool );
  this->setAttribute( Qt::WA_MacAlwaysShowToolWindow, true );
}

VisDisplayBase::~VisDisplayBase()
{
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
  mUserIsMoving = false;
  mTitle = inConfig[ CfgID::WindowTitle ];
  if( !mTitle.empty() )
    this->setWindowTitle( mTitle.c_str() );
  else
    this->setWindowTitle( mSourceID.c_str() );

  // The static variables make each new window appear a little down right
  // from the previous one.
  static int newTop = 10,
             newLeft = 10;
  int formTop = 10,
      formLeft = 10,
      formHeight = 100,
      formWidth = 100;
  bool posDefault  = !inConfig.Get( CfgID::Top, formTop ) ||
                     !inConfig.Get( CfgID::Left, formLeft ),
       sizeDefault = !inConfig.Get( CfgID::Height, formHeight ) ||
                     !inConfig.Get( CfgID::Width, formWidth );
  if( posDefault )
  {
    formTop = newTop;
    newTop += 10;
    formLeft = newLeft;
    newLeft += 10;
  }
  if( formWidth <= 10 || formHeight <= 10 )
  {
    sizeDefault = true;
    formHeight = 100;
    formWidth = 100;
  }
  int desktopWidth = QApplication::desktop()->width(),
      desktopHeight = QApplication::desktop()->height();
  if( formTop < 0 || formLeft < 0 
      || formTop >= desktopHeight || formLeft >= desktopWidth )
  {
    posDefault = true;
    formTop = newTop;
    newTop += 10;
    formLeft = newLeft;
    newLeft += 10;
  }
  this->move( formLeft, formTop );
  this->resize( formWidth, formHeight );
  if( posDefault )
  {
    Visconfigs()[ mSourceID ].Put( CfgID::Top, formTop, Default );
    Visconfigs()[ mSourceID ].Put( CfgID::Left, formLeft, Default );
  }
  if( sizeDefault )
  {
    Visconfigs()[ mSourceID ].Put( CfgID::Width, formWidth, Default );
    Visconfigs()[ mSourceID ].Put( CfgID::Height, formHeight, Default );
  }

  bool visible = true;
  inConfig.Get( CfgID::Visible, visible );
  this->setVisible( visible );
  mUserIsMoving = true;
}

void
VisDisplayBase::Restore()
{
  SetConfig( Visconfigs()[ mSourceID ] );
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
VisDisplayBase::HandleProperty( const char* inVisID, VisDisplay::IDType inCfgID, const char* inValue, ConfigState inState )
{
  Visconfigs()[ inVisID ].Put( inCfgID, inValue, inState );
  if( Visuals()[ inVisID ] != NULL )
    Visuals()[ inVisID ]->SetConfig( Visconfigs()[ inVisID ] );
}

void
VisDisplayBase::moveEvent( QMoveEvent* iopEvent )
{
  if( mUserIsMoving )
  {
    Visconfigs()[ mSourceID ].Put( CfgID::Top, this->pos().y(), UserDefined );
    Visconfigs()[ mSourceID ].Put( CfgID::Left, this->pos().x(), UserDefined );
  }
  QWidget::moveEvent( iopEvent );
}

void
VisDisplayBase::resizeEvent( QResizeEvent* iopEvent )
{
  this->update();
  if( mUserIsMoving )
  {
    Visconfigs()[ mSourceID ].Put( CfgID::Width, this->size().width(), UserDefined );
    Visconfigs()[ mSourceID ].Put( CfgID::Height, this->size().height(), UserDefined );
  }
  QWidget::resizeEvent( iopEvent );
}

