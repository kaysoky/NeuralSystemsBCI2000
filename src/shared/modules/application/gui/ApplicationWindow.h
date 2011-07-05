////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class for application windows. Typically, there is only one
//   instance of this class in an application module, but there may be more
//   than one.
//   The ApplicationWindow class takes care of
//   * parameterizing window position and size,
//   * displaying a miniature copy of the window in an operator visualization
//     window,
//   * updating changed portions of the window after processing, and writing
//     the StimulusTime time stamp immediately after updating.
//
//   ApplicationWindow instances are associated with names, and may be
//   retrieved via the ApplicationWindow::Windows() container, specifying
//   the name as a key: ApplicationWindow::Windows()["Application"].
//   Names must be unique across instances of ApplicationWindow within an
//   application module, but may be reused across application modules.
//   An instance's name is specified as an argument to the ApplicationWindow
//   constructor; when no name is given, it defaults to "Application".
//
//   When multiple ApplicationWindow instances are present, the StimulusTime
//   time stamp will reflect the time when the last window was updated.
//
//   Parameter and visualization names are derived from the instance's name.
//   The default name, "Application", is treated as a special case, and
//   results in the following names historically used for application windows:
//     WindowLeft, WindowTop, WindowWidth, WindowHeight for positioning
//     parameters;
//     VisualizeApplicationWindow, AppWindowSpatialDecimation, 
//     AppWindowTemporalDecimation for visualization parameters;
//     ApplicationWindow as the visualization ID.
//   For all other window names, dependent names are constructed as follows:
//     <name>WindowLeft, <name>WindowTop, <name>WindowWidth, <name>WindowHeight;
//     Visualize<name>Window, <name>WindowSpatialDecimation, 
//     <name>WindowTemporalDecimation;
//     <name>Window as the visualization ID.
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
#ifndef APPLICATION_WINDOW_H
#define APPLICATION_WINDOW_H

#include "DisplayWindow.h"
#include "Environment.h"
#include "GenericVisualization.h"
#include <map>

class ApplicationWindow : public GUI::DisplayWindow, private EnvironmentExtension
{
 public:
  static std::string DefaultName;

  ApplicationWindow( const std::string& = DefaultName );
  virtual ~ApplicationWindow();

  // EnvironmentExtension interface
  virtual void Publish();
  virtual void Preflight() const;
  virtual void Initialize();
  virtual void PostInitialize();
  virtual void StartRun();
  virtual void PostStopRun();
  virtual void PostProcess();

  // Properties
  const std::string& Name() const
  { return mName; }

 private:
  std::string mName;
  struct
  {
    std::string Left,
                Top,
                Width,
                Height,
                BackgroundColor,
                Visualize,
                SpatialDecimation,
                TemporalDecimation;
  } mParamNames;

  BitmapVisualization      mVis;
  BitmapImage              mImageBuffer;
  bool                     mDoVisualize;
  int                      mWidth,
                           mHeight,
                           mTemporalDecimation,
                           mBlockCount;

 public:
  static ApplicationWindowList& Windows();
};


class ApplicationWindowList : public std::map<std::string, ApplicationWindow*>
{
 public:
  bool IsEmpty() const
       { return empty(); }
  int  Size() const 
       { return size(); }
  bool Exists( const std::string& s ) const
       { return find( s ) != end(); }
  ApplicationWindow*& operator[]( const std::string& );
  ApplicationWindow* operator[]( const std::string& ) const;
  ApplicationWindow* operator[]( int ) const;
};


#endif // APPLICATION_WINDOW_H
