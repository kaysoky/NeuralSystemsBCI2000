/////////////////////////////////////////////////////////////////////////////
// $Id: WebcamLogger.h 2118 6/30/2010
// Authors: adam.wilson@uc.edu
// Description: The Webcam Logger logs video data from a standard webcam,
// saves the video in a compressed video format, and stores the frame number
// as a state
//
// Parameterization:
//   Eyetracker logging is enabled from the command line adding
//     --LogEyetracker=1
//   As a command line option.
//   Eyetracker is given an external calibration file through
//     CalibrationData - calibration file
//   WebcamLogger needs the Network address and port of Eyetracker
//     NetworkLocation - network address of eyetracker
//     Port - incoming port for eyetracker traffic
//   User may also set the desired interval in ms between polls
//     Interval - approx ms between data polling
//   Information may be selectively logged by setting these parameters to true
//     LogGazeData - record gaze data to states
//     LogEyePosition - record eye position to states
//     LogPupilSize - record pupil size to states
//     LogEyeDist - record eye distances to states
//
// State Variables:
//   WebcamFrameNumber
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
/////////////////////////////////////////////////////////////////////////////
#ifndef WEBCAM_LOGGER_H
#define WEBCAM_LOGGER_H

#include <opencv2/highgui/highgui.hpp>
//#include "cv.h"
#include <string>
#include <iostream>
#include <QDateTime>
#include <QString>
#include "Environment.h"
#include "OSThread.h"
#include "OSMutex.h"
#include "GenericVisualization.h"
#include "PrecisionTime.h"

using namespace std;

class WebcamLogger : public EnvironmentExtension
{
public:
	//Constructors and virtual interface
	WebcamLogger();
	virtual ~WebcamLogger();
	virtual void Publish();
	virtual void Preflight() const;
	virtual void Initialize();
	virtual void Process();
	virtual void StartRun();
	virtual void StopRun();
	virtual void Halt();

private:
	
	bool mWebcamEnable;
	int mCamNum;
	string mOutputFile;	
	int mDecimation;
	int mTextLocation;
	bool mResting;

	class WebcamThread : public OSThread
	{
	public:
		WebcamThread( WebcamLogger* logger );
		virtual ~WebcamThread();
		virtual int Execute();
		void Initialize();
		void Reset();
		void SetRunning( bool val ) { mRunning = val; }
		//bool Disabled() { return mDisabled; }
		void GetFrame();
		void SetResting(bool val);

	private:
		bool           mRunning; //Disables Callbacks
		bool           mDisabled; 
		int            mCamNum;
		bool mResting;
		WebcamLogger* mWebcam;
		string mOutputFile;
		CvCapture *mCapture;
		CvVideoWriter *mWriter;
		IplImage  *mFrame;
		unsigned int mFrameNum;
		bool mIsVisible;
		void *mWinHandle;
		CvFont mFont;
		CvPoint mTextLoc;
		int mTargetWidth, mTargetHeight, mSourceWidth, mSourceHeight;
		unsigned short mT1;
		int mCnt;
		int mDecimation;
		int mTextLocation;
		void AddDateTime();
		float mFps;
		float mTargetFps;
		OSMutex mMutex;
		void InitText();
		void InitCam();

	} *mpWebcamThread;

};

#endif // WEBCAM_LOGGER_H





