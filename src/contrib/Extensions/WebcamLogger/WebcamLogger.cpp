/////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: adam.wilson@uc.edu
// Description: The WebcamLogger records webcam video to an AVI file
// and sets the frame number as a state value
//
// Parameterization:
//   Webcam logging is enabled from the command line adding
//     --LogWebcam=1
//   As a command line option.
//
// Event Variables:
//   WebcamFrame - The current frame number stored as a state
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
/////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "WebcamLogger.h"
#include "FileUtils.h"
#include "BCIEvent.h"

#include <cstring>
#include <ctime>
#include <iomanip>
#include <iostream>

using namespace std;

Extension( WebcamLogger );

static time_t Now()
{ return ::time( 0 ); }

static string TimeToString( time_t inTime )
{ // format is "MM/dd/yy hh:mm:ss"
  struct ::tm t = { 0 },
             *p = ::localtime( &inTime );
  if( !p )
    return "<n/a>";

  t = *p;
  ostringstream oss;
  oss << setfill( '0' )
      << setw( 2 ) << t.tm_mon + 1 << '/'
      << setw( 2 ) << t.tm_mday << '/'
      << setw( 2 ) << t.tm_year % 100 << ' '
      << setw( 2 ) << t.tm_hour << ':'
      << setw( 2 ) << t.tm_min << ':'
      << setw( 2 ) << t.tm_sec;
  return oss.str();
}

// **************************************************************************
// Function:   WebcamLogger
// Purpose:    This is the constructor for the WebcamLogger class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
WebcamLogger::WebcamLogger()
:mWebcamEnable( false ) //, m_logLocked( false )
{
	//m_log.flush();
	mpWebcamThread = NULL;
	mCamNum = 0;
}

// **************************************************************************
// Function:   ~WebcamLogger
// Purpose:    This is the destructor for the WebcamLogger class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
WebcamLogger::~WebcamLogger()
{
	if (mpWebcamThread)
		delete mpWebcamThread;
}

// **************************************************************************
// Function:   Publish
// Purpose:    This function requests parameters by adding parameters to the
//             parameter list it also requests states by adding them to the
//             state list
// Returns:    N/A
// **************************************************************************
void WebcamLogger::Publish()
{
	bool webcamEnable = false;
	webcamEnable = ( ( int )OptionalParameter( "LogWebcam" ) != 0 );

	if( !webcamEnable ){
		if (mpWebcamThread) delete mpWebcamThread;
		return;
	}

	BEGIN_PARAMETER_DEFINITIONS
		"Source:Webcam int LogWebcam= 1 0 0 1"
		" // allow logging from webcam (boolean)",
		"Source:Webcam int CameraNumber= 0 0 0 %"
		" // the webcam number for systems with multiple cameras",
		"Source:Webcam int WebcamDecimation= 1 1 1 %"
		" // save/display every nth frame",
		"Source:Webcam int WebcamDateTimeLocation= 0 0 0 4"
		" // date/time text location in saved video: "
			" 0: none,"
			" 1: UpperRight,"
			" 2: UpperLeft,"
			" 3: LowerRight,"
			" 4: LowerLeft"
			" (enumeration)",
		END_PARAMETER_DEFINITIONS

		BEGIN_EVENT_DEFINITIONS
		"WebcamFrame 24 0 0 0",
		END_EVENT_DEFINITIONS
}



// **************************************************************************
// Function:   Preflight
// Purpose:    Checks parameters for availability and consistency with
//             input signal properties; requests minimally needed properties for
//             the output signal; checks whether resources are available.
// Parameters: Input and output signal properties.
// Returns:    N/A
// **************************************************************************
void WebcamLogger::Preflight() const
{
	bool webcamEnable = false;
	int camNum = 0;

	webcamEnable = ( ( int )OptionalParameter( "LogWebcam" ) != 0 );

	if( webcamEnable && mpWebcamThread==NULL)
	{
		camNum = (int)Parameter("CameraNumber");

		//initialize the camera
		CvCapture *capture = NULL;
		bciout << "Initializing Webcam..."<< endl;
		capture = cvCreateCameraCapture(camNum);
		if (!capture){
			bcierr<<"Cannot initialize webcam with index " << camNum <<". Please make sure that the webcam is connected, and that the Camera Number is appropriate."<<endl;
			return;
		}
		//calculate frame rate over 10 frames

		IplImage *frame;
		frame = cvQueryFrame( capture );
		int nFrames = 10;
		PrecisionTime t1 = PrecisionTime::Now();
		for (int i = 0; i < nFrames; i++)
		{
			frame = cvQueryFrame( capture );
		}
		PrecisionTime t2 = PrecisionTime::UnsignedDiff(PrecisionTime::Now(),t1);
		float fps = 1000.0f/(float(t2)/nFrames);
		bciout << "Avg. webcam framerate="<<fps<<endl;

		//disconnect the camera
		cvReleaseCapture(&capture);
		Parameter("WebcamDecimation");
		Parameter("WebcamDateTimeLocation");
	}

	Parameter("DataDirectory");
	Parameter("SubjectName");
	Parameter("SubjectSession");
	Parameter("SubjectRun");
}

// **************************************************************************
// Function:   Initialize
// Purpose:    This function parameterizes the WebcamLogger
// Parameters: Input and output signal properties.
// Returns:    N/A
// **************************************************************************
void WebcamLogger::Initialize()
{
	mWebcamEnable = ( ( int )OptionalParameter( "LogWebcam" ) != 0 );
	if(mWebcamEnable)
	{
		mCamNum = (int)Parameter("CameraNumber");
		mTextLocation = (int)Parameter("WebcamDateTimeLocation");
		mDecimation = (int)Parameter("WebcamDecimation");

		if (!mpWebcamThread){
			mpWebcamThread = new WebcamThread( this );
		}
		else{
			mpWebcamThread->Initialize();
		}
	}
}


// **************************************************************************
// Function:   Process
// Purpose:    Checks the thread and updates the Validity Log
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void WebcamLogger::Process()
{
}

// **************************************************************************
// Function:   StartRun
// Purpose:    Starts a new webcam thread at the beginning of a run
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void WebcamLogger::StartRun()
{
	if( mWebcamEnable )
	{
    mOutputFile = CurrentRun();
    mOutputFile = FileUtils::ExtractDirectory( mOutputFile ) + FileUtils::ExtractBase( mOutputFile ) + "_vid.avi";
		mpWebcamThread->SetResting(false);
		//mpWebcamThread = new WebcamThread( this );
	}
}

// **************************************************************************
// Function:   StopRun
// Purpose:    Terminates the Eyetracker thread at the end of a run
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void WebcamLogger::StopRun()
{
	if (mpWebcamThread)
		mpWebcamThread->SetResting(true);
}

// **************************************************************************
// Function:   Halt
// Purpose:    Stops all asynchronous operation
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void WebcamLogger::Halt()
{
	if( mpWebcamThread )
	{
		mpWebcamThread->SetRunning( false );
		mpWebcamThread->TerminateWait();
		delete mpWebcamThread;
		bcidbg( 10 ) << "4. Destroy Thread" << endl;
		mpWebcamThread = NULL;
	}
}



// **************************************************************************
// Function:   EyetrackerThread constructor
// Purpose:    Initializes the Eyetracker thread
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
WebcamLogger::WebcamThread::WebcamThread(WebcamLogger* logger)
: mCamNum(0),
mOutputFile(""),
mDecimation(1),
mTextLocation(0)
{
	//Initialize everything to it's proper value
	mRunning = true;
	mDisabled = false;
	mWebcam = logger;
	mFrame = NULL;
	mWriter = NULL;
	mCapture = NULL;
	mIsVisible = true;
	mTargetWidth = 320;
	mTargetHeight = 240;
	mSourceWidth = 640;
	mSourceHeight = 480;

	mOutputFile = mWebcam->mOutputFile;
	mDecimation = mWebcam->mDecimation;
	mResting = true;

	Initialize();

	cvSetCaptureProperty(mCapture, CV_CAP_PROP_FPS, mFps);
	CvSize size = cvSize(mTargetWidth,mTargetHeight);

	cvNamedWindow( "result", CV_WINDOW_NORMAL | CV_GUI_NORMAL );
	mWinHandle = cvGetWindowHandle("result");
	mTargetFps = mFps/mDecimation;

	this->Start();
}

// **************************************************************************
// Function:   EyetrackerThread destructor
// Purpose:    Cleans up the Eyetracker thread
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
WebcamLogger::WebcamThread::~WebcamThread()
{
	if (mCapture){
		cvReleaseCapture(&mCapture);
		mCapture=NULL;
	}

	if (mWriter){
		cvReleaseVideoWriter(&mWriter);
		mWriter = NULL;
	}

	if (mWinHandle){
		cvDestroyWindow("result");
		mWinHandle = NULL;
	}
}

void WebcamLogger::WebcamThread::Initialize()
{
	if (mCamNum != mWebcam->mCamNum || mCapture==NULL){
		mCamNum = mWebcam->mCamNum;
		InitCam();
	}
	if (mTextLocation != mWebcam->mTextLocation){
		mTextLocation = mWebcam->mTextLocation;
		InitText();
	}
}

void WebcamLogger::WebcamThread::Reset()
{
}


void WebcamLogger::WebcamThread::InitText()
{
	if (mTextLocation > 0)
	{
		cvInitFont(&mFont, CV_FONT_HERSHEY_SIMPLEX,0.5,0.5,0,1,CV_AA);
		string dt = TimeToString( Now() );
		CvSize textSize;
		int baseline;
		cvGetTextSize(dt.c_str(), &mFont, &textSize, &baseline);
		switch (mTextLocation){
			case 1: //UR
				mTextLoc = cvPoint(mSourceWidth-10 - textSize.width,10+textSize.height);
				break;
			case 2: //UL
				mTextLoc = cvPoint(10,10+textSize.height);
				break;
			case 3: //LR
				mTextLoc = cvPoint(mSourceWidth-10 - textSize.width,  mSourceHeight-10-baseline);
				break;
			case 4: //LL
				mTextLoc = cvPoint(10, mSourceHeight-10-baseline);
				break;
			default:
				mTextLoc = cvPoint(10,10);
				break;
		}
	}
}

void WebcamLogger::WebcamThread::InitCam()
{
	if (mCapture){
		cvReleaseCapture(&mCapture);
		mCapture=NULL;
	}
	mCapture = cvCaptureFromCAM( mCamNum );
	if (mCapture == NULL){
		bcierr << "Error connecting to webcam." <<endl;
		mRunning = false;
		return;
	}

	int nFrames = 10;
	mFrame = cvQueryFrame( mCapture );
	PrecisionTime t1 = PrecisionTime::Now();
	for (int i = 0; i < nFrames; i++)
	{
		mFrame = cvQueryFrame( mCapture );
	}
	PrecisionTime t2 = PrecisionTime::UnsignedDiff(PrecisionTime::Now(), t1);
	mFps = 1000.0f/(float(t2)/nFrames);
}

void WebcamLogger::WebcamThread::SetResting(bool val)
{
	if (val != mResting && !val)
	{
		//start recording
		mOutputFile = mWebcam->mOutputFile;
		if (mTextLocation != mWebcam->mTextLocation)
		{
			//update text
			InitText();
		}
		mTextLocation = mWebcam->mTextLocation;

		if (mCamNum != mWebcam->mCamNum)
		{
			InitCam();
		}
		mCamNum = mWebcam->mCamNum;

		if (mDecimation != mWebcam->mDecimation)
		{
			mTargetFps = mFps/mDecimation;
		}
		mDecimation = mWebcam->mDecimation;

		CvSize size = cvSize(mTargetWidth,mTargetHeight);
		if (mWriter){
			mMutex.Acquire();
			cvReleaseVideoWriter(&mWriter);
			mWriter = NULL;
			mMutex.Release();
		}

		if (mOutputFile.size() > 0){
			mWriter = cvCreateVideoWriter(mOutputFile.c_str(),CV_FOURCC('D','I','V','X'),mTargetFps,size,1);
		}
		mFrameNum = 0;
		mResting = val;
		mCnt = 0;
	}
	else
	{
		mResting = val;
		if (mWriter){
			mMutex.Acquire();
			cvReleaseVideoWriter(&mWriter);
			mWriter = NULL;
			mMutex.Release();
		}
		mFrameNum = 0;
		mCnt = 0;
	}

}

// **************************************************************************
// Function:   Execute
// Purpose:    This is the Eyetracker thread function
// Parameters: N/A
// Returns:    Always zero
// **************************************************************************
int WebcamLogger::WebcamThread::Execute()
{
	mFrameNum = 0;
	mCnt = 0;
	mIsVisible = true;
	while( !this->IsTerminating() && mRunning )
	{
		this->GetFrame();
	}

	return 0;
}

void WebcamLogger::WebcamThread::GetFrame()
{
	int ret;
	mFrame = cvQueryFrame( mCapture );
	if( mFrame ){
		if ((mCnt % mDecimation)==0){
			if (mTextLocation > 0)
				AddDateTime();
			if (mWriter && mFrame && !mResting){
				mMutex.Acquire();
				ret = cvWriteFrame(mWriter, mFrame);
				mMutex.Release();
				bcievent << "WebcamFrame " << ++mFrameNum;
			}
			if (mIsVisible && mWinHandle)
				cvShowImage( "result", mFrame );
		}
		mCnt++;
	}
	else
	{
		if( !mResting ) bcievent << "WebcamFrame " << 0;
	}
}

void WebcamLogger::WebcamThread::AddDateTime()
{
	string dt = TimeToString( Now() );//QDateTime::currentDateTime().toString("MM/dd/yy hh:mm:ss");
	cvPutText(mFrame, dt.c_str(), mTextLoc, &mFont, cvScalar(0,0,255,0));

}
