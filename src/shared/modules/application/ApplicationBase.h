////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A base class for application modules.
//         This class defines parameters common to all application modules, and
//         defines two output streams intended for logging purposes:
//         - The AppLog.Screen stream is directed into a window displayed to the
//           operator user.
//         - The AppLog.File stream is directed into a log file, and displayed
//           in the operator user's log window.
//         - Writing to AppLog results in logging both to screen and file.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef APPLICATION_BASE_H
#define APPLICATION_BASE_H

#include "GenericFilter.h"
#include "GenericVisualization.h"
#include "LogFile.h"
#include "RandomGenerator.h"
#include "GraphDisplay.h"
#include <iostream>
#include <set>

class ApplicationBase : public GenericFilter
{
 protected:
  ApplicationBase( const GUI::GraphDisplay* = NULL );

 public:
  virtual ~ApplicationBase() {}

  virtual void Preflight( const SignalProperties& Input,
                                SignalProperties& Output ) const = 0;
  virtual void Initialize( const SignalProperties& Input,
                           const SignalProperties& Output ) = 0;

  virtual void Process( const GenericSignal& Input,
                              GenericSignal& Output ) = 0;
  virtual void Halt() {}

  virtual bool AllowsVisualization() const { return false; }

 protected:
  RandomGenerator RandomNumberGenerator;
  
  class StreamBundle : public std::ostream
  {
   public:
    StreamBundle()
      : std::ostream( 0 ),
        mBuf( mStreams )
      { this->init( &mBuf ); }
    ~StreamBundle()
      {}

    StreamBundle& Add( std::ostream& os )
      { mStreams.insert( &os ); return *this; }

   private:
    typedef std::set<std::ostream*> StreamSet;
    StreamSet mStreams;

    class BundleStringbuf : public std::stringbuf
    {
     public:
      BundleStringbuf( const StreamSet& streams )
        : std::stringbuf( std::ios_base::out ),
          mStreams( streams )
        {}
     private:
      virtual int sync();
      const StreamSet& mStreams;
    } mBuf;
  };
  // The struct declaration allows for writing
  //  AppLog.File   << "This goes into the file";
  //  AppLog.Screen << "This goes to the screen";
  //  AppLog        << "This goes to both file and screen";
  struct AppLogStruct: public StreamBundle
  {
    AppLogStruct()
      : File( ".applog" ),
        Screen( "APLG" )
      { Add( File ).Add( Screen ); }

    LogFile              File;
    GenericVisualization Screen;
  } AppLog;

 private:
  class DisplayVisualization : public EnvironmentExtension
  {
   public:
    DisplayVisualization( const GUI::GraphDisplay* );

    virtual void Publish()
      {}
    virtual void Preflight() const;
    virtual void Initialize() {}
    virtual void PostInitialize();
    virtual void StartRun();
    virtual void PostStopRun();
    virtual void PostProcess();

   private:
    void SendReferenceFrame();
    void SendDifferenceFrame();

    const GUI::GraphDisplay* mpDisplay;
    GenericVisualization     mVis;
    BitmapImage              mImageBuffer;
    bool                     mDoVisualize;
    int                      mWidth,
                             mHeight,
                             mTemporalDecimation,
                             mBlockCount;
  } mDisplayVis;
};

#endif // APPLICATION_BASE_H
