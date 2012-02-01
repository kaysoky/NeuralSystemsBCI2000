////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class for application windows. For more information, see
//   the associated header file.
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

#include "ApplicationWindow.h"
#include "PrecisionTime.h"
#include "BCIError.h"

using namespace std;
using namespace GUI;

string ApplicationWindow::DefaultName = "Application";

ApplicationWindow::ApplicationWindow( const std::string& inName )
: mName( inName ),
  mVis( inName + "Window" ),
  mDoVisualize( false ),
  mWidth( 0 ),
  mHeight( 0 ),
  mTemporalDecimation( 0 ),
  mBlockCount( 0 )
{
  if( mName == DefaultName )
  {
    mParamNames.Left = "WindowLeft";
    mParamNames.Top = "WindowTop";
    mParamNames.Width = "WindowWidth";
    mParamNames.Height = "WindowHeight";
    mParamNames.BackgroundColor = "WindowBackgroundColor";
    mParamNames.Visualize = "VisualizeApplicationWindow";
    mParamNames.SpatialDecimation = "AppWindowSpatialDecimation";
    mParamNames.TemporalDecimation = "AppWindowTemporalDecimation";
  }
  else
  {
    mParamNames.Left = mName + "WindowLeft";
    mParamNames.Top = mName + "WindowTop";
    mParamNames.Width = mName + "WindowWidth";
    mParamNames.Height = mName + "WindowHeight";
    mParamNames.BackgroundColor = mName + "WindowBackgroundColor";
    mParamNames.Visualize = string( "Visualize" ) + mName + "Window";
    mParamNames.SpatialDecimation = mName + "WindowSpatialDecimation";
    mParamNames.TemporalDecimation = mName + "WindowTemporalDecimation";
    GUI::DisplayWindow::SetTitle( string( "BCI2000 " ) + mName );
  }

  if( Windows().find( mName ) != Windows().end() )
    bcierr << "A window with name \"" << mName 
           << "\" already exists in application module"
           << endl;
  else
    Windows()[mName] = this;
}

ApplicationWindow::~ApplicationWindow()
{
  if( Windows()[mName] == this )
    Windows().erase( mName );
}


void
ApplicationWindow::Publish()
{
  struct { const char* id, * value; } variables[] =
  {
    { "$name$", mName.c_str() },
    { "$width$", mParamNames.Width.c_str() },
    { "$height$", mParamNames.Height.c_str() },
    { "$left$", mParamNames.Left.c_str() },
    { "$top$", mParamNames.Top.c_str() },
    { "$background$", mParamNames.BackgroundColor.c_str() },
    { "$visualize$", mParamNames.Visualize.c_str() },
    { "$spatialdecimation$", mParamNames.SpatialDecimation.c_str() },
    { "$temporaldecimation$", mParamNames.TemporalDecimation.c_str() },
  };
  const char* parameters[] =
  {
    "Application:$name$%20Window int $width$= 640 640 0 % "
      " // width of $name$ window",
    "Application:$name$%20Window int $height$= 480 480 0 % "
      " // height of $name$ window",
    "Application:$name$%20Window int $left$= 0 0 % % "
      " // screen coordinate of $name$ window's left edge",
    "Application:$name$%20Window int $top$= 0 0 % % "
      " // screen coordinate of $name$ window's top edge",
    "Application:$name$%20Window string $background$= 0x000000 0x505050 % % "
      "// $name$ window background color (color)",

    "Visualize:$name$%20Window int $visualize$= 0 0 0 1 "
      "// Display miniature copy of $name$ window (boolean)",
    "Visualize:$name$%20Window int $spatialdecimation$= 8 8 1 % "
      "// $name$ window decimation (shrinking) factor",
    "Visualize:$name$%20Window int $temporaldecimation$= 4 16 1 % "
      "// $name$ window time decimation factor",
  };
  for( size_t i = 0; i < sizeof( parameters ) / sizeof( *parameters ); ++i )
  {
    string param = parameters[i];
    for( size_t j = 0; j < sizeof( variables ) / sizeof( *variables ); ++j )
    {
      size_t pos;
      do
      {
        pos = param.find( variables[j].id );
        if( pos != string::npos )
          param = param.substr( 0, pos ) + variables[j].value + param.substr( pos + ::strlen( variables[j].id ) );
      } while( pos != string::npos );
    }
    BEGIN_PARAMETER_DEFINITIONS
      param.c_str(),
    END_PARAMETER_DEFINITIONS
  }

  BEGIN_STATE_DEFINITIONS
    "StimulusTime 16 0 0 0",
  END_STATE_DEFINITIONS
}

void
ApplicationWindow::Preflight() const
{
  Parameter( mParamNames.Left );
  Parameter( mParamNames.Top );
  Parameter( mParamNames.BackgroundColor );
}

void
ApplicationWindow::Initialize()
{
  DisplayWindow::SetLeft( Parameter( mParamNames.Left ) );
  DisplayWindow::SetTop( Parameter( mParamNames.Top ) );
  DisplayWindow::SetWidth( Parameter( mParamNames.Width ) );
  DisplayWindow::SetHeight( Parameter( mParamNames.Height ) );
  DisplayWindow::SetColor( RGBColor( Parameter( mParamNames.BackgroundColor ) ) );

  mDoVisualize = ( Parameter( mParamNames.Visualize ) > 0 );
  if( mDoVisualize )
  {
    mTemporalDecimation = Parameter( mParamNames.TemporalDecimation );
    int spatialDecimation = Parameter( mParamNames.SpatialDecimation );
    mWidth = static_cast<int>( ( DisplayWindow::Context().rect.right - DisplayWindow::Context().rect.left ) / spatialDecimation );
    mHeight = static_cast<int>( ( DisplayWindow::Context().rect.bottom - DisplayWindow::Context().rect.top ) / spatialDecimation );
    mVis.Send( CfgID::WindowTitle, mName + " Window" );
  }
  mVis.Send( CfgID::Visible, mDoVisualize );
}

void
ApplicationWindow::PostInitialize()
{
  DisplayWindow::Show();
  DisplayWindow::Update();
  if( mDoVisualize )
    mVis.SendReferenceFrame( DisplayWindow::BitmapData( mWidth, mHeight ) );
}

void
ApplicationWindow::StartRun()
{
  mBlockCount = mTemporalDecimation - 1;
}

void
ApplicationWindow::PostStopRun()
{
  DisplayWindow::Update();

  if( mDoVisualize )
    mVis.SendReferenceFrame( DisplayWindow::BitmapData( mWidth, mHeight ) );
}

void
ApplicationWindow::PostProcess()
{
  DisplayWindow::Update();
  State( "StimulusTime" ) = PrecisionTime::Now();

  if( mDoVisualize && ( ++mBlockCount %= mTemporalDecimation ) == 0 )
    mVis.SendDifferenceFrame( DisplayWindow::BitmapData( mWidth, mHeight ) );
}

#ifdef __BORLANDC__
# pragma warn -8104 // No warning about local statics.
#endif // __BORLANDC__

ApplicationWindowList&
ApplicationWindow::Windows()
{
  static ApplicationWindowList instance;
  return instance;
}

// ApplicationWindowList definitions
ApplicationWindow*&
ApplicationWindowList::operator[]( const std::string& inName )
{
  return map<string, ApplicationWindow*>::operator []( inName );
}

ApplicationWindow*
ApplicationWindowList::operator[]( const std::string& inName ) const
{
  const_iterator i = find( inName );
  return i == end() ? NULL : i->second;
}

ApplicationWindow*
ApplicationWindowList::operator[]( int inIdx ) const
{
  const_iterator i = begin();
  int j = 0;
  while( j < inIdx && i != end() )
    ++j, ++i;
  return i == end() ? NULL : i->second;
}

// ApplicationWindowClient definitions
const ApplicationWindowList* const ApplicationWindowClient::Windows = &ApplicationWindow::Windows();

ApplicationWindowClient::ApplicationWindowClient()
{
}

ApplicationWindowClient::~ApplicationWindowClient()
{
  for( WindowSet::const_iterator i = mWindowsAccessed.begin(); i != mWindowsAccessed.end(); ++i )
  {
    ( *i )->UnregisterUser( this );
    if( ( *i )->Users() == 0 )
      delete *i;
  }
}

ApplicationWindow&
ApplicationWindowClient::Window( const string& inName ) const
{
  string name = inName;
  if( name.empty() )
    name = ApplicationWindow::DefaultName;

  ApplicationWindow* pWindow = ( *Windows )[name];
  if( pWindow == NULL )
  { // New Windows are legally created on access during the construction
    // phase. Outside the construction phase, we report an error, but still
    // return a valid reference to allow the caller to proceed.
    pWindow = new ApplicationWindow( name );
    if( Environment::Phase() != Environment::construction )
      bcierr_ << "Access to non-existent application window \""
              << name
              << "\""
              << endl;
  }
  pWindow->RegisterUser( this );

  switch( Environment::Phase() )
  {
    case Environment::construction:
    case Environment::preflight:
      break;

    default:
      if( mWindowsAccessed.find( pWindow ) == mWindowsAccessed.end() )
        bcierr_ << "Application window \""
                << name
                << "\" was neither declared during construction, "
                << "nor tested for existence during preflight phase."
                << endl;
  }
  mWindowsAccessed.insert( pWindow );

  return *pWindow;
}



