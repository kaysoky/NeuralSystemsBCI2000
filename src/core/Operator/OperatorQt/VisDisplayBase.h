////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
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
#ifndef VIS_DISPLAY_BASE_H
#define VIS_DISPLAY_BASE_H

#include "VisDisplay.h"

#include <QWidget>
#include <map>
#include <string>
#include <sstream>

class GenericSignal;
class BitmapImage;

class VisDisplayBase : public QWidget
{
  Q_OBJECT

  friend class VisDisplay;
    
 public:
  typedef enum // Possible states of properties ("configs").
  {
    Default = 0,     // May be overridden by a message or by user settings.
    OnceUserDefined, // A previous user setting, read from the registry.
    UserDefined,     // Set by the user.
    MessageDefined,  // Set by a message, overrides user.
  } ConfigState;

 protected:
  VisDisplayBase( const std::string& visID );
 public:
  virtual ~VisDisplayBase();

  static void SetParentWindow( QWidget* w ) { spParentWindow = w; }
  static void Clear() { Visuals().Clear(); }
  static void HandleSignal( const char* visID, const GenericSignal& );
  static void HandleMemo( const char* visID, const char* );
  static void HandleBitmap( const char* visID, const BitmapImage& );
  static void HandleProperty( const char* visID, VisDisplay::IDType cfgID, const char* value, ConfigState );

 protected:
  virtual void Restore();
  virtual void Save() const;

 protected:
  void moveEvent( QMoveEvent* );
  void resizeEvent( QResizeEvent* );

 protected:
  std::string mSourceID;
 private:
  std::string mTitle;
  bool mUserIsMoving;

 protected:
  static QWidget* spParentWindow;

 protected:
  // visID->display instance
  typedef std::map< std::string, VisDisplayBase* > VisContainerBase;
  class VisContainer : public VisContainerBase
  {
     public:
    ~VisContainer() { Clear(); }
    void Clear();
  };
  static VisContainer& Visuals();

 protected:
  // configID->value
  typedef std::map< VisDisplay::IDType, std::string > ConfigSettingsBase;
  class ConfigSettings : public ConfigSettingsBase
  {
   public:
    template<typename T> bool Get( VisDisplay::IDType id, T& t, ConfigState minState = Default );
    template<typename T> bool Put( VisDisplay::IDType id, const T& t, ConfigState state );
    ConfigState& State( VisDisplay::IDType id ) { return mStates[ id ]; }

   private:
    std::map< VisDisplay::IDType, ConfigState > mStates;
  };

  // visID->config information
  class ConfigContainer : public std::map< std::string, ConfigSettings >
  {
   public:
    ConfigContainer()  { Restore(); }
    ~ConfigContainer() { Save(); }
    void Save();
    void Restore();
  };
  static ConfigContainer& Visconfigs();
  virtual void SetConfig( ConfigSettings& );
};

template<typename T>
bool
VisDisplayBase::ConfigSettings::Get( VisDisplay::IDType id, T& t, ConfigState minState )
{
  const_iterator i = find( id );
  if( i == end() )
    return false;
  if( State( id ) < minState )
    return false;
  std::istringstream is( i->second );
  if( is.str() != "" )
  {
    T value;
    if( is >> value )
      t = value;
  }
  return !is.fail();
}

template<typename T>
bool
VisDisplayBase::ConfigSettings::Put( VisDisplay::IDType id, const T& t, ConfigState state )
{
  if( State( id ) > state )
    return false;
  State( id ) = state;
  std::ostringstream os;
  os << t;
  ( *this )[ id ] = os.str();
  return !os.fail();
}

#endif // VIS_DISPLAY_BASE_H
