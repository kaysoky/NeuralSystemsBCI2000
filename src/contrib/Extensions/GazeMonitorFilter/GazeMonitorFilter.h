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

#include "GenericFilter.h"
#include "WavePlayer.h"
#include "Expression/Expression.h"
#include "GenericVisualization.h"
#include "GraphDisplay.h"
#include "GraphObject.h"
#include "ImageStimulus.h"
#include "TextField.h"
#include "Shapes.h"

class GazeMonitorFilter : public GenericFilter
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

 private:
  // Private methods
  void InitSound( std::string &filename, WavePlayer* wp ) const; 
  void SetDisplayRect( GUI::GraphObject* obj, float cx, float cy, float rad );
  void DeleteStimuli();
  void ViolatedFixation();
  void AcquiredFixation();

  // Private member variables
  bool mEnforceFixation;
  Expression mFixationX;
  Expression mFixationY;
  float mFixationRadius;
  WavePlayer* mpViolationSound;
  bool mVisualizeGaze;
  bool mLogGazeInformation;
  bool mFixated;
  float mOffset;
  float mScale;
  bool mLoggingGaze;
  bool mLoggingEyePos;
  bool mLoggingEyeDist;
  int mCorrection;
  bool mLostLeftEye;
  bool mLostRightEye;
  float mAspectRatio;
  int mBlinkTime;
  int mBlinkBlocks;
  int mSaccadeTime;
  int mSaccadeBlocks;
  int mTemporalDecimation;
  int mBlockCount;

  enum Displays {
    APP = 0,
    VIS,
    NUM_DISPLAYS
  };

  // Visual Elements
  ImageStimulus* mpFixationImage;
  ImageStimulus* mpFixationViolationImage;
  TextField*     mpPrompt;
  EllipticShape* mpRightEye[NUM_DISPLAYS];
  EllipticShape* mpLeftEye[NUM_DISPLAYS]; 
  EllipticShape* mpCursor[NUM_DISPLAYS];
  EllipticShape* mpZone[NUM_DISPLAYS];

  GUI::GraphDisplay* mpDisplays[NUM_DISPLAYS]; 
  BitmapVisualization mVis;
};

#endif // GAZE_MONITOR_FILTER_H
