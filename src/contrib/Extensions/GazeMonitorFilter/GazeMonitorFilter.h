////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: griffin.milsap@gmail.com
// Description: A helper filter which acts on Gaze data from an eyetracker
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef GAZE_MONITOR_FILTER_H
#define GAZE_MONITOR_FILTER_H

#include "ApplicationBase.h"
#include "WavePlayer.h"
#include "Expression/Expression.h"
#include "GenericVisualization.h"
#include "GraphDisplay.h"
#include "GraphObject.h"
#include "ImageStimulus.h"
#include "TextField.h"
#include "Shapes.h"

class GazeMonitorFilter : public ApplicationBase
{
 public:
  // Constructor/Destructor
  GazeMonitorFilter();
  ~GazeMonitorFilter();

 protected:
  // Virtual Interface
  virtual void Preflight( const SignalProperties& Input,
                                SignalProperties& Output ) const;
  virtual void Initialize( const SignalProperties& Input,
                           const SignalProperties& Output );
  virtual void Process( const GenericSignal& Input,
                              GenericSignal& Output );
  virtual void StartRun();
  virtual void StopRun();
  virtual void Halt();

 private:
  // Private methods
  void InitSound( const std::string& inFilename, WavePlayer& ioPlayer) const; 
  void SetDisplayRect( GUI::GraphObject* obj, float cx, float cy, float rad );
  void DeleteStimuli();
  void ViolatedFixation();
  void AcquiredFixation();

  // Private member variables
  bool mEnforceFixation, mVisualizeGaze;
  Expression mFixationX, mFixationY;
  float mFixationRadius;
  WavePlayer mViolationSound;
  bool mLogGazeInformation;
  bool mFixated;
  float mOffset, mScale;
  float mLastGazeX, mLastGazeY;
  bool mLoggingEyetracker,
       mLoggingGaze,
       mLoggingEyePos,
       mLoggingEyeDist;
  int mCorrection;
  bool mLostLeftEye, mLostRightEye;
  float mAspectRatio;
  int mBlinkTime, mBlinkBlocks;
  int mSaccadeTime, mSaccadeBlocks;
  int mTemporalDecimation, mBlockCount;

  // Visual Elements
  ImageStimulus* mpFixationImage;          // App screen only
  ImageStimulus* mpFixationViolationImage; // App screen only
  TextField*     mpPrompt;                 // App screen only
  EllipticShape* mpZone;                   // App screen only

  EllipticShape* mpRightEye;               // Vis screen only
  EllipticShape* mpLeftEye;                // Vis screen only
  EllipticShape* mpGaze;                   // Vis screen only

  GUI::GraphDisplay mVisDisplay;
  GUI::GraphDisplay* mpAppDisplay;
  BitmapVisualization mVis;
};

#endif // GAZE_MONITOR_FILTER_H
