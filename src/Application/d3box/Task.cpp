/*************************************************************************
Task.cpp for the d3box task
*************************************************************************/
#include <vcl.h>
#pragma hdrstop

#include "Task.h"
#include "Usr.h"
#include "BCIDirectry.h"
#include "UState.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dos.h>
#include <math.h>

//#define  DATAGLOVE

RegisterFilter( TTask, 3 );

/*shidong starts*/
FILE *b;        //debug file used to track the function flow in this file
bool printFlow;
/*shidong ends*/

TTask::TTask()
: run( 0 ),
  vis( NULL ),
  appl( NULL ),
#ifdef DATAGLOVE
  my_glove( new DataGlove ),
#endif
  OldRunning( 0 ),
  OldCurrentTarget( 0 )
{
  BEGIN_PARAMETER_DEFINITIONS

    "3DEnvironment int CameraX = 0 0 0 32767 // "
      "Camera X coordinate in the unit of +- 32767",
    "3DEnvironment int CameraY = 0 0 0 32767 // "
      "Camera Y coordinate in the unit of +- 32767",
    "3DEnvironment int CameraZ = 40959 40959 0 32767 // "
      "Camera Z coordinate in the unit of +- 32767",
    "3DEnvironment int CameraAimX = 0 0 0 32767 // "
      "Camera Aim X coordinate in the unit of +- 32767",
    "3DEnvironment int CameraAimY = 0 0 0 32767 // "
      "Camera Aim Y coordinate in the unit of +- 32767",
    "3DEnvironment int CameraAimZ = 0 0 0 32767 // "
      "Camera Aim Z coordinate in the unit of +- 32767",
    "3DEnvironment int LightSourceX = 0 0 0 32767 // "
      "Light Source X coordinate in the unit of +- 32767",
    "3DEnvironment int LightSourceY = 0 0 0 32767 // "
      "Light Source Y coordinate in the unit of +- 32767",
    "3DEnvironment int LightSourceZ = 32767 0 0 32767 // "
      "Light Source Z coordinate in the unit of +- 32767",
    "3DEnvironment int LightSourceColorR = 127 0 0 255 // "
      "Light Source Color's Red Value, range from 0~255",
    "3DEnvironment int LightSourceColorG = 127 0 0 255 // "
      "Light Source Color's Green Value, range from 0~255",
    "3DEnvironment int LightSourceColorB = 127 0 0 255 // "
      "Light Source Color's Blue Value, range from 0~255",
    "3DEnvironment int LightSourceIntensity = 255 0 0 255 // "
      "Light Source Color's Intensity Value, range from 0~255",


    "TrackingTask int UseTracking= 1 0 0 0 // "
      "Enable tracking task (1=yes, 0=target task)",
    "TrackingTask float EllipseRadiusX= 14000 0 0 0 // "
      "X radius of ellipse",
    "TrackingTask float EllipseRadiusY= 14000 0 0 0 // "
      "Y radius of ellipse",
    "TrackingTask float EllipseRadiusDecrX= 0.5 0 0 0 // "
      "continuously decreases X radius of ellipse",
    "TrackingTask float EllipseRadiusDecrY= 0.5 0 0 0 // "
      "continuously decreases Y radius of ellipse",
    "TrackingTask float TrackingSpeed= 0.02 0 0 0 // "
      "Adv. of tracking cursor per sample block (no units)",
    "TrackingTask int TrackingShape= 1 0 0 0 // "
      "1=ellipse, 2=triangle",
    "TrackingTask float TriangleSizeX= 5 0 0 0 // "
      "Size of triangle in X",
    "TrackingTask float TriangleSizeY= 5 0 0 0 // "
      "Size of triangle in Y",


    "UsrTask int PreTrialPause= 10 0 0 0 // "
      "Duration of Target w/o cursor",
    "UsrTask int ItiDuration= 10 0 0 0 // "
      "Duration of Intertrial Interval",
    "UsrTask int RewardDuration= 10 0 0 0 // "
      "Duration of PostTrial Feedback",

    "UsrTask int FeedbackDuration= 20000 0 0 0 // "
      "Max Trial Duration",

    "UsrTask int BaselineInterval= 1 0 0 2 // "
      "Intercept Computation 0 = none 1 = targets 2 = ITI (enumeration)",
    "UsrTask int TimeLimit= 180 180 0 1000 // "
      "Time Limit for Runs in seconds",
    "UsrTask int RestingPeriod= 0 0 0 1 // "
      "rest period of data acquisition (boolean)",
    "UsrTask int WorkspaceBoundaryVisible = 1 0 0 1"
      "Show the Workspace boundary (boolean)",

    "UsrTask string BorderTexture= C:/Documents%20and%20Settings/shidong/My%20Documents/BCI2000/BCIJuly20/Application/shared/3DAPI/texture/Glass.bmp a z //"
      "Path of border texture (inputfile)",
    "UsrTask string TargetTexture= C:/Documents%20and%20Settings/shidong/My%20Documents/BCI2000/BCIJuly20/Application/shared/3DAPI/texture/Cube.bmp a z //"
      "Path of target texture (inputfile)",
    "UsrTask string CursorTexture= C:/Documents%20and%20Settings/shidong/My%20Documents/BCI2000/BCIJuly20/Application/shared/3DAPI/texture/Glass.bmp a z //"
      "Path of cursor texture (inputfile)",
    "UsrTask float CursorSize= 2 0 0 1 // "
      "User Window Cursor Size",
    "UsrTask int CursorColorFront = 0x000000 0x000000 0x000000 0xFFFFFF//"
       "Cursor color when it is at the front of the workspace in the format of 0xRRGGBB (color)",
    "UsrTask int CursorColorBack = 0x000000 0x000000 0x000000 0xFFFFFF//"
       "Cursor color when it is at the front of the workspace in the format of 0xRRGGBB (color)",
    "UsrTask int WinHeight= 512 0 0 1 // "
        "User Window Height",
    "UsrTask int WinWidth= 512 0 0 1 // "
        "User Window Width",
    "UsrTask int WinXpos= 600 0 0 1 // "
        "User Window X location",
    "UsrTask int WinYpos= 5 0 0 1 // "
        "User Window Y location",
    "UsrTask int WindowFullScreen = 0 0 0 1 // "
        "Full screen mode (0=no, 1=yes) (boolean)",
    "UsrTask int DisplayMonitor = 0 0 0 1 // "
        "Check for secondary display, uncheck for primary display(0=no, 1=yes) (boolean)",
    "UsrTask int ChangeResolution = 0 0 0 1 // "
        "Apply changes in screen resolution (0=no, 1=yes) (boolean)",

  #ifdef DATAGLOVE
    "JoyStick string GloveCOMport= COM2 0 % % // "
      "COM port for 5DT glove",
    "JoyStick int UseJoyStick= 0 0 0 2 // "
      "0=brain signals; 1=Joystick; 2=Mouse; 3=Glove (enumeration)",
  #else // DATAGLOVE
    "JoyStick int UseJoyStick= 0 0 0 1 // "
      "0=brain signals; 1=Joystick; 2=Mouse (enumeration)",
  #endif // DATAGLOVE
    "JoyStick float JoyXgain= 4.0 0 -1000.0 1000.0 // "
      "Horizontal gain",
    "JoyStick float JoyYgain= 4.0 0 -1000.0 1000.0 // "
      "Vertical gain",
    "JoyStick float JoyZgain= 1.0 0 -1000.0 1000.0 // "
      "Depth gain, should always be 1 to avoid float overflow error",
    "JoyStick float XOffset= 0 0 -1000.0 1000.0 // "
      "horizontal offset for joystick/glove",
    "JoyStick float YOffset= 0 0 -1000.0 1000.0 // "
      "horizontal offset for joystick/glove",
    "JoyStick float ZOffset= 0 0 -1000.0 1000.0 // "
      "depth offset for joystick/glove",

    "Targets int NumberTargets= 4 0 0 0 // "
      "Number of Targets",
    "Targets int IncludeAllTargets= 0 0 0 1 // "
      "Test all target positions? 1=test all targets. 0=test only the visible current target"
      " (enumeration)",
    "Targets float StartCursorX= 50.0 0 0 100.0 // "
      "Horizontal Start of Cursor",
    "Targets float StartCursorY= 50.0 0 0 100.0 // "
      "Vertical Cursor Starting Position",
    "Targets float StartCursorZ= 50.0 0 0 100.0 // "
      "Depth Cursor Starting Position",

    "Targets matrix TargetPos= 10 6"
        "35 65 35 65 100 0 "
        "35 35 65 65 100 0 "
        "50 50 50 50 100 0 "
        "8  8  8  8   8  8 "
        "8  8  8  8   8  8 "
        "8  8  8  8   8  8 "
        "0  0 -1  1   0  0 "
        "-1 1  0  0   0  0 "
        "0  0  0  0   0  0 "
        "0  0  0  0   0  0 "// Target Position Matrix - Values are 0-100",


  #ifdef DATAGLOVE
    "JoyStick matrix GloveControlX= ",
      "{t-1 t sign} ", // row labels
      "{ thumb index middle ring little pitch roll } ", // column labels
      " -1 0 0 0 0 0 0 ",
      "  1 0 0 0 0 0 0 ",
      "  1 1 1 1 1 1 1 ",
      " 0 0 0 // glove sensor weights for horizontal movement",

    "JoyStick matrix GloveControlY= ",
      "{t-1 t sign} ", // row labels
      "{ thumb index middle ring little pitch roll } ", // column labels
      " 0 0 -1 0 0 0 0 ",
      " 0 0  1 0 0 0 0 ",
      " 1 1  1 1 1 1 1 ",
      " 0 0 0 // glove sensor weights for vertical movement",
  #endif // DATAGLOVE
  END_PARAMETER_DEFINITIONS

  BEGIN_STATE_DEFINITIONS
  #ifdef DATAGLOVE
    "GloveSensor1 8 0 0 0",
    "GloveSensor2 8 0 0 0",
    "GloveSensor3 8 0 0 0",
    "GloveSensor4 8 0 0 0",
    "GloveSensor5 8 0 0 0",
    "GloveSensor6 8 0 0 0",
    "GloveSensor7 8 0 0 0",
  #endif // DATAGLOVE

    "TargetCode 5 0 0 0",
    "ResultCode 5 0 0 0",
    "ResponseTime 8 0 0 0",
    "StimulusTime 16 17528 0 0",
    "Feedback 2 0 0 0",
    "IntertrialInterval 2 1 0 0",
    "RestPeriod 2 0 0 0",
    "CursorPosX 16 0 0 0",
    "CursorPosY 16 0 0 0",
    "CursorPosZ 16 0 0 0",
    "TrkCursorPosX 16 0 0 0",
    "TrkCursorPosY 16 0 0 0",
    "TrkCursorPosZ 16 0 0 0",

    "Xadapt 16 0 0 0",
    "Yadapt 16 0 0 0",
    "Zadapt 16 0 0 0",
    "AdaptCode 5 0 0 0",
  END_STATE_DEFINITIONS

  SetUsr( Parameters, States );

  printFlow=false;
  if(printFlow)
  {
          /*shidong starts   */
          if (  (b = fopen("TTaskFlow.txt", "w")) == NULL )
          {
                printFlow = false;
                bcierr << "Could not open TTaskFlow.txt for writing" << std::endl;
          }
          else
          {
                printFlow = true;
          }
  }
 /*  shidong ends*/
if(printFlow) fprintf(b, "In TTask::Constructor.\n");
}

//-----------------------------------------------------------------------------


/*helper function
int TTask::hex2dec( AnsiString hex )
{
        if ( hex.SubString(0,1) == "-") //if the hex is a negative
        {
                AnsiString sub = hex.SubString(2, hex.Length());
                return 0-sub.ToInt();
        }
        else
        {
                return hex.ToInt();
        }
}//hex2dec    */



TTask::~TTask( void )
{
/*shidong starts*/
if(printFlow) fprintf(b, "In TTask::~TTask function.\n");
/*shidong ends*/
        if( vis ) delete vis;
        #ifdef DATAGLOVE
        delete my_glove;
        #endif
        vis= NULL;
        if (appl) fclose( appl );
/*shidong starts*/
if (b) fclose(b);
/*shidong ends*/

}

void TTask::checkPathHelper()
{

        borderTexture = ( const char*  )Parameter( "BorderTexture" );
        targetTexture = ( const char*  )Parameter( "TargetTexture" );
        cursorTexture = ( const char* )Parameter( "CursorTexture" );
        if ( (checkPath(borderTexture)) == false)
        {
                bciout << "The file in path: " << borderTexture.c_str() << " is either not exist or error occurred when loading the file as texture"  << std::endl;
                borderTexture = "";
        }

        if ( (checkPath(targetTexture)) == false)
        {
                bciout << "The file in path: " << targetTexture.c_str() << " is either not exist or error occurred when loading the file as texture"  << std::endl;
                targetTexture = "";
        }

        if ( (checkPath(cursorTexture)) == false)
        {
                bciout << "The file in path: " << cursorTexture.c_str() << " is either not exist or error occurred when loading the file as texture"  << std::endl;
                cursorTexture = "";
        }
}//checkPathHelper

//check the input path to see if it is a valid file
bool TTask::checkPath(AnsiString path)
{
        try
        {
                if(path.Length() == 0)
                {
                        return false;
                }
                else
                {
                        FILE *File=NULL;        // File Handle
	                File=fopen(path.c_str(),"r");   // Check To See If The File Exists
	                if (File)		// Does The File Exist?
	                {
		                fclose(File);   // Close The Handle
                                if (auxDIBImageLoad(path.c_str())==NULL)
		                {
                                        return false;
                                }
                                else
                                {
                                        return true;
                                }
	                }
                        else
                        {
                                return false;
                        }

                }
        }
        catch(...)
        {
                return false;
        }
}//checkpath

//check input string to see if it is a legal int
bool TTask::checkInt(AnsiString input, AnsiString paraName) const
{
        try
        {
                input.ToDouble();    //check to see input is a valid number or not
        }
        catch(...)      //any error happen
        {
                bcierr << "The parameter " << paraName.c_str() << " is not a valid input.  It is likely that \"" << input.c_str() << "\" is not a numeric number.  It will be default to 0." <<std::endl;
                return false;
        }
        return true;            //catch nothin, input is a valid numeric string
}//checkInt

void TTask::Preflight( const SignalProperties& inputProperties,
                             SignalProperties& outputProperties )const
{
/*shidong starts*/
if(printFlow) fprintf(b, "In TTask::Preflight function.\n");
/*shidong ends*/


  // External parameters.
  Parameter( "FileInitials" );
  Parameter( "SubjectSession" );
  Parameter( "SubjectName" );

  // External states.
  State( "IntertrialInterval" );
  State( "Running" );

  // TTask::Process() implies that the input signal has at least two integer channels
  // with one element each.
  PreflightCondition( inputProperties >= SignalProperties( 2, 1, SignalType::int16 ) );

  /*shidong starts*/
  //Parameter checking

        /***Check The UskTask Tab***/

        //Check CursorColor
        AnsiString str1, str2, sub1, sub2;
        str1 =  ( const char* )Parameter( "CursorColorFront");
        str2 =  ( const char* )Parameter( "CursorColorBack");
        sub1 =  str1.SubString(3, str1.Length() );
        sub2 =  str2.SubString(3, str2.Length() );
        if (str1.Length() != COLORFORMAT || str1.SubString(1,1)!= "0" || str1.SubString(2,1)!= "x")      //if format is not 0xRRGGBB
        {
                bcierr << "The format of CursorColorFront is not correct. It should be 0xRRGGBB" << std::endl;
        }
        if (str2.Length() != COLORFORMAT || str1.SubString(1,1)!= "0" || str2.SubString(2,1)!= "x")      //if format is not 0xRRGGBB
        {
                bcierr << "The format of CursorColorBack is not correct. It should be 0xRRGGBB" << std::endl;
        }
        if ( str1.ToIntDef(-1) < 0 || str1.ToIntDef(-1) > 0xFFFFFF || str1.ToIntDef(-1) == -1)
        {
                bcierr << "The value of the CursorColorFront should be between 0 and 0xFFFFFF.  Input is " << str1.ToIntDef(-1) << std::endl;
        }
        if ( str2.ToIntDef(-1) < 0 || str2.ToIntDef(-1) > 0xFFFFFF || str2.ToIntDef(-1) == -1)
        {
                bcierr << "The value of the CursorColorFront should be between 0 and 0xFFFFFF.  Input is " << str2.ToIntDef(-1) << std::endl;
        }
        //check the path of the texture
        checkPathHelper();
        //Integer paramter of the UsrTask tab
        checkInt((const char*)Parameter("BaseLineInterval"), "BaseLineInterval");
        checkInt((const char*)Parameter("CursorSize"), "CursorSize");
        checkInt((const char*)Parameter("FeedbackDuration"), "FeedbackDuration");
        checkInt((const char*)Parameter("ItiDuration"), "ItiDuration");
        checkInt((const char*)Parameter("PreTrialPause"), "PreTrialPause");
        checkInt((const char*)Parameter("RestingPeriod"), "RestingPeriod");
        checkInt((const char*)Parameter("RewardDuration"), "RewardDuration");
        checkInt((const char*)Parameter("TimeLimit"), "TimeLimit");
        checkInt((const char*)Parameter("WinHeight"), "WinHeight");
        checkInt((const char*)Parameter("WinWidth"), "WinWidth");
        checkInt((const char*)Parameter("WinXpos"), "WinXpos");
        checkInt((const char*)Parameter("WinYpos"), "WinYpos");
        checkInt((const char*)Parameter("WorkspaceBoundaryVisible"), "WorkspaceBoundaryVisible");
        PreflightCondition( Parameter("WorkspaceBoundaryVisible")== 0 || Parameter("WorkspaceBoundaryVisible")== 1);
        PreflightCondition( Parameter("BaseLineInterval")== 1 || Parameter("BaseLineInterval")== 2);


        /***Check The Visualize Tab**
        checkInt((const char*)Parameter("SourceMax"), "SourceMax");
        checkInt((const char*)Parameter("SourceMin"), "SourceMin");
        checkInt((const char*)Parameter("VisualizeCalibration"), "VisualizeCalibration");
        checkInt((const char*)Parameter("VisualizeClassFiltering"), "VisualizeClassFiltering");
        checkInt((const char*)Parameter("VisualizeNormalFiltering"), "VisualizeNormalFiltering");
        checkInt((const char*)Parameter("VisualizeRoundtrip"), "VisualizeRoundtrip");
        checkInt((const char*)Parameter("VisualizeSource"), "VisualizeSource");
        checkInt((const char*)Parameter("VisualizeSourceDecimation"), "VisualizeSourceDecimation");
        checkInt((const char*)Parameter("VisualizeSourceTime"), "VisualizeSourceTime");
        checkInt((const char*)Parameter("VisualizeSpatialFiltering"), "VisualizeSpatialFiltering");
        checkInt((const char*)Parameter("VisualizeStatFiltering"), "VisualizeStatFiltering");
        checkInt((const char*)Parameter("VisualizeTemporalFiltering"), "VisualizeTemporalFiltering");
        PreflightCondition( Parameter("VisualizeCalibration")== 1 || Parameter("VisualizeCalibration")== 0);
        PreflightCondition( Parameter("VisualizeClassFiltering")== 1 || Parameter("VisualizeClassFiltering")== 0);
        PreflightCondition( Parameter("VisualizeNormalFiltering")== 1 || Parameter("VisualizeNormalFiltering")== 0);
        PreflightCondition( Parameter("VisualizeRoundtrip")== 1 || Parameter("VisualizeRoundtrip")== 0);
        PreflightCondition( Parameter("VisualizeSource")== 1 || Parameter("VisualizeSource")== 0);
        PreflightCondition( Parameter("VisualizeSpatialFiltering")== 1 || Parameter("VisualizeSpatialFiltering")== 0);
        PreflightCondition( Parameter("VisualizeStatFiltering")== 1 || Parameter("VisualizeStatFiltering")== 0);
        PreflightCondition( Parameter("VisualizeTemporalFiltering")== 1 || Parameter("VisualizeTemporalFiltering")== 0);
        */
        /***Check The Joystick Tab***/
        checkInt((const char*)Parameter("JoyXgain"), "JoyXgain");
        checkInt((const char*)Parameter("JoyYgain"), "JoyYgain");
        checkInt((const char*)Parameter("JoyZgain"), "JoyZgain");
        checkInt((const char*)Parameter("UseJoyStick"), "UseJoyStick");
        checkInt((const char*)Parameter("XOffset"), "XOffset");
        checkInt((const char*)Parameter("YOffset"), "XOffset");
        checkInt((const char*)Parameter("ZOffset"), "ZOffset");
        PreflightCondition( Parameter("UseJoyStick") >= 0 && Parameter("UseJoyStick") <= 3);
        if (Parameter("JoyZgain") != 1)
                bciout << "JoyZgain should be set to 1 to avoid float point overflow." << std::endl;

        /***Check The Targets Tab***/
        checkInt((const char*)Parameter("IncludeAllTargets"), "IncludeAllTargets");
        checkInt((const char*)Parameter("NumberTargets"), "NumberTargets");
        checkInt((const char*)Parameter("StartCursorX"), "StartCursorX");
        checkInt((const char*)Parameter("StartCursorY"), "StartCursorY");
        checkInt((const char*)Parameter("StartCursorZ"), "StartCursorZ");
        PreflightCondition( Parameter("IncludeAllTargets")== 1 || Parameter("IncludeAllTargets")== 0);
        PreflightCondition( Parameter("TargetPos")->GetNumColumns() == Parameter("NumberTargets"));
        PreflightCondition( Parameter("TargetPos")->GetNumRows() == 10);

        /***Check The Storage Tab***/
        checkInt((const char*)Parameter("SavePrmFile"), "BaseLineInterval");
        //FileInitial will be checked in the Initialize.
        //Subject Name will checked with FileInitial in Initialize
        //StorageTime is only used in EEG.
        PreflightCondition( Parameter("SavePrmFile")== 1 || Parameter("SavePrmFile")== 0);
        PreflightCondition( ((AnsiString) (const char*)Parameter("SubjectRun")).Length() ==2);
        PreflightCondition( ((AnsiString) (const char*)Parameter("SubjectSession")).Length() ==3);

        /***Check The MEMFilter Tab*
        checkInt((const char*)Parameter("deltaMem"), "deltaMem");
        checkInt((const char*)Parameter("MemBandWidth"), "MemBandWidth");
        checkInt((const char*)Parameter("MemDetrend"), "MemDetrend");
        checkInt((const char*)Parameter("MemModelOrder"), "MemModelOrder");
        checkInt((const char*)Parameter("MemWindows"), "MemWindows");
        checkInt((const char*)Parameter("StartMem"), "StartMem");
        checkInt((const char*)Parameter("StopMem"), "StopMem");
        PreflightCondition( Parameter("MemDetrend")== 1 || Parameter("MemDetrend")== 0 || Parameter("MemDetrend")== 2);
            **/
        /***Check The Source Tab*
        checkInt((const char*)Parameter("DCoffset"), "DoTrueRandom");
        checkInt((const char*)Parameter("DoTrueRandom"), "DoTrueRandom");
        checkInt((const char*)Parameter("ModulateAmplitude"), "ModulateAmplitude");
        checkInt((const char*)Parameter("NoiseMaxAmplitude"), "NoiseMaxAmplitude");
        checkInt((const char*)Parameter("NoiseMinAmplitude"), "NoiseMinAmplitude");
        checkInt((const char*)Parameter("SampleBlockSize"), "SampleBlockSize");
        checkInt((const char*)Parameter("SamplingRate"), "SamplingRate");
        checkInt((const char*)Parameter("SineChannel"), "SineChannel");
        checkInt((const char*)Parameter("SineChannelX"), "SineChannelX");
        checkInt((const char*)Parameter("SineFrequency"), "SineFrequency");
        checkInt((const char*)Parameter("SineMaxAmplitude"), "SineMaxAmplitude");
        checkInt((const char*)Parameter("SineMinAmplitude"), "SineMinAmplitude");
        checkInt((const char*)Parameter("SoftwareCh"), "SoftwareCh");
        checkInt((const char*)Parameter("TransmitCh"), "TransmitCh");
        //checkInt((const char*)Parameter("TransmitChList"), "TransmitChList");
        PreflightCondition( Parameter("DoTrueRandom")== 1 || Parameter("DoTrueRandom")== 0);
        PreflightCondition( Parameter("ModulateAmplitude")== 1 || Parameter("ModulateAmplitude")== 0);

           **/
        /***Check The 3DEnvironment Tab***/

        checkInt((const char*)Parameter("CameraX"), "CameraX");
        checkInt((const char*)Parameter("CameraY"), "CameraY");
        checkInt((const char*)Parameter("CameraZ"), "CameraZ");
        checkInt((const char*)Parameter("LightSourceX"), "LightSourceX");
        checkInt((const char*)Parameter("LightSourceY"), "LightSourceY");
        checkInt((const char*)Parameter("LightSourceZ"), "LightSourceZ");
        checkInt((const char*)Parameter("CameraAimX"), "CameraAimX");
        checkInt((const char*)Parameter("CameraAimY"), "CameraAimY");
        checkInt((const char*)Parameter("CameraAimZ"), "CameraAimZ");
        checkInt((const char*)Parameter("LightSourceColorR"), "LightSourceColorR");
        checkInt((const char*)Parameter("LightSourceColorG"), "LightSourceColorG");
        checkInt((const char*)Parameter("LightSourceColorB"), "LightSourceColorB");
        checkInt((const char*)Parameter("LightSourceIntensity"), "LightSourceIntensity");
        PreflightCondition( Parameter("CameraAimX")>= -32767  && Parameter("CameraAimX")<= 32767);
        PreflightCondition( Parameter("CameraAimY")>= -32767  && Parameter("CameraAimY")<= 32767);
        PreflightCondition( Parameter("CameraAimZ")>= -32767  && Parameter("CameraAimZ")<= 32767);
        PreflightCondition( Parameter("LightSourceColorR")>= 0  && Parameter("LightSourceColorR")<= 255);
        PreflightCondition( Parameter("LightSourceColorG")>= 0  && Parameter("LightSourceColorG")<= 255);
        PreflightCondition( Parameter("LightSourceColorB")>= 0  && Parameter("LightSourceColorB")<= 255);
        PreflightCondition( Parameter("LightSourceIntensity")>= 0  && Parameter("LightSourceIntensity")<= 255);


        /***Check The Statistics Tab**
        checkInt((const char*)Parameter("DesiredPixelsPerSec"), "DesiredPixelsPerSec");
        checkInt((const char*)Parameter("HorizInterceptProp"), "HorizInterceptProp");
        checkInt((const char*)Parameter("HorizTrendControl"), "HorizTrendControl");
        checkInt((const char*)Parameter("InterceptControl"), "InterceptControl");
        checkInt((const char*)Parameter("InterceptLength"), "InterceptLength");
        checkInt((const char*)Parameter("InterceptProportion"), "InterceptProportion");
        checkInt((const char*)Parameter("LinTrendLrnRt"), "LinTrendLrnRt");
        checkInt((const char*)Parameter("LRPixelsPerSec"), "LRPixelsPerSec");
        checkInt((const char*)Parameter("QuadTrendLrnRt"), "QuadTrendLrnRt");
        checkInt((const char*)Parameter("TrendControl"), "TrendControl");
        checkInt((const char*)Parameter("TrendWinLth"), "TrendWinLth");
        checkInt((const char*)Parameter("WeightUse"), "WeightUse");
        checkInt((const char*)Parameter("WtLrnRt"), "WtLrnRt");
        PreflightCondition( Parameter("HorizTrendControl")== 1  || Parameter("HorizTrendControl") == 2 || Parameter("HorizTrendControl") == 0);
        PreflightCondition( Parameter("InterceptControl")== 1  || Parameter("InterceptControl") == 2 || Parameter("InterceptControl") == 0);
        PreflightCondition( Parameter("TrendControl")== 1  || Parameter("TrendControl") == 2 || Parameter("TrendControl") == 0);
        PreflightCondition( Parameter("WeightUse")== 1  || Parameter("WeightUse") == 2 || Parameter("WeightUse") == 0);
                              */
        /***Check The System Tab**
        checkInt((const char*)Parameter("ApplicationPort"), "ApplicationPort");
        checkInt((const char*)Parameter("EEGSourcePort"), "EEGSourcePort");
        checkInt((const char*)Parameter("SignalProcessingPort"), "SignalProcessingPort");
        checkInt((const char*)Parameter("StateVectorLength"), "StateVectorLength");
        PreflightCondition( Parameter("ApplicationPort")>= 0 );
        PreflightCondition( Parameter("EEGSourcePort")>= 0 );
        PreflightCondition( Parameter("SignalProcessingPort")>= 0 );

        /***Check The Filtering Tab**

        checkInt((const char*)Parameter("AlignChannels"), "AlignChannels");
        checkInt((const char*)Parameter("ClassMode"), "ClassMode");
        checkInt((const char*)Parameter("LR_A"), "LR_A");
        checkInt((const char*)Parameter("LR_B"), "LR_B");
        checkInt((const char*)Parameter("NumControlSignals"), "NumControlSignals");
        checkInt((const char*)Parameter("SourceChTimeOffset"), "SourceChTimeOffset");
        checkInt((const char*)Parameter("SpatialFilteredChannels"), "SpatialFilteredChannels");
        checkInt((const char*)Parameter("UD_A"), "NUD_A");
        checkInt((const char*)Parameter("UD_B"), "NUD_B");
        PreflightCondition( Parameter("AlignChannels")== 1 || Parameter("AlignChannels")== 0);
        PreflightCondition( Parameter("ClassMode")== 1 || Parameter("ClassMode")== 2);     */
/*shidong ends*/




  #ifdef DATAGLOVE
  // test for presence of glove if we want to use the glove
  if (Parameter( "UseJoystick" ) == 3)
     {
     PreflightCondition( my_glove->GlovePresent(AnsiString((const char *)Parameter("GloveCOMport"))) );
     PreflightCondition( Parameter("GloveControlX")->GetNumRows() == 3);
     PreflightCondition( Parameter("GloveControlX")->GetNumColumns() == MAX_GLOVESENSORS);
     PreflightCondition( Parameter("GloveControlY")->GetNumRows() == 3);
     PreflightCondition( Parameter("GloveControlY")->GetNumColumns() == MAX_GLOVESENSORS);
     }
  #endif

  // We connect the input signal through to the output signal.
  outputProperties = inputProperties;
}


void TTask::Initialize()
{
/*shidong starts*/
if(printFlow) fprintf(b, "In TTask::Initialize function.\n");
/*shidong ends*/
TEMPORARY_ENVIRONMENT_GLUE
        AnsiString FInit,SSes,SName,AName;
        AnsiString COMport;
        /*shidong starts*/

        AnsiString str1, str2;
        /*shidong ends*/
        time_t ctime;
        struct tm *tblock;
        int i,j;

        PtpDuration=       Parameter( "PreTrialPause" );
        ItiDuration=       Parameter( "ItiDuration" );
        OutcomeDuration=   Parameter( "RewardDuration" );
        FeedbackDuration=  Parameter( "FeedbackDuration" );
        Ntargets=          Parameter( "NumberTargets" );
        CursorStartX=      Parameter( "StartCursorX" );
        CursorStartY=      Parameter( "StartCursorY" );
        CursorStartZ=      Parameter( "StartCursorZ" );
        targetInclude=     Parameter( "IncludeAllTargets" );
        BaselineInterval=  Parameter( "BaselineInterval" );
        Resting=           Parameter( "RestingPeriod" );
        CursorRadius=      Parameter( "CursorSize" );
        timelimit=         Parameter( "TimeLimit" );
        WinHeight=         Parameter( "WinHeight");
        WinWidth=          Parameter( "WinWidth");
        WinXpos =          Parameter( "WinXpos");
        WinYpos=           Parameter( "WinYpos");
        /*shidong starts*/
        CameraX                 = Parameter( "CameraX");
        CameraY                 = Parameter( "CameraY");
        CameraZ                 = Parameter( "CameraZ");
        CameraAimX              = Parameter( "CameraAimX");
        CameraAimY              = Parameter( "CameraAimY");
        CameraAimZ              = Parameter( "CameraAimZ");
        LightSourceX            = Parameter( "LightSourceX");
        LightSourceY            = Parameter( "LightSourceY");
        LightSourceZ            = Parameter( "LightSourceZ");
        LightSourceColorR       = Parameter( "LightSourceColorR");
        LightSourceColorG       = Parameter( "LightSourceColorG");
        LightSourceColorB       = Parameter( "LightSourceColorB");
        LightSourceIntensity    = Parameter( "LightSourceIntensity");
        WindowFullScreen        = Parameter( "WindowFullScreen");
        DisplayMonitor          = Parameter( "DisplayMonitor");
        ChangeResolution        = Parameter( "ChangeResolution");

        TrackingTarget=false;
        if (Parameter("UseTracking") == 1)
           {
           TrackingTarget=true;
           ellipse_radius_x = Parameter( "EllipseRadiusX");
           ellipse_radius_y = Parameter( "EllipseRadiusY");
           ellipse_radius_decrement_x = Parameter( "EllipseRadiusDecrX");
           ellipse_radius_decrement_y = Parameter( "EllipseRadiusDecrY");
           tracking_speed = Parameter("TrackingSpeed");
           tracking_shape = Parameter("TrackingShape");
           triangle_size_x = Parameter("TriangleSizeX");
           triangle_size_y = Parameter("TriangleSizeY");
           }

        User->PutTrackingTarget( &x_pos, &y_pos, clBlack );

        WorkspaceBoundaryVisible = Parameter( "WorkspaceBoundaryVisible");


        CursorColorFront =  ( const char* )Parameter( "CursorColorFront");
        CursorColorBack =  ( const char* )Parameter( "CursorColorBack");
        if(printFlow) fprintf(b, "The CursorColorFront is %s.\n", CursorColorFront);
        if(printFlow) fprintf(b, "The CursorColorBack  is %s.\n", CursorColorBack);


        /*shidong ends*/
        FInit= ( const char* )Parameter( "FileInitials" );
        SSes = ( const char* )Parameter( "SubjectSession" );
        SName= ( const char* )Parameter( "SubjectName" );

        useJoy= Parameter( "UseJoyStick" );
        #ifdef DATAGLOVE
        if ((useJoy < 0) || (useJoy > 3))
          bcierr << "UseJoyStick has to be 0, 1, 2, or 3" << std::endl;
        #else
        if ((useJoy < 0) || (useJoy > 2))
          bcierr << "UseJoyStick has to be 0, 1, or 2" << std::endl;
        #endif

        #ifdef DATAGLOVE
        // transfer the weights for glove into local variables
        if (useJoy == 3)
           {
           COMport= ( const char* )Parameter( "GloveCOMport" );
           for (int sensor=0; sensor<MAX_GLOVESENSORS; sensor++)
            {
            for (int i=0; i<3; i++)
             {
             GloveControlX[i][sensor]=Parameter("GloveControlX", i, sensor);
             GloveControlY[i][sensor]=Parameter("GloveControlY", i, sensor);
             }
            AnsiString glovestatename="GloveSensor"+AnsiString(sensor+1);
            State(glovestatename.c_str())=0;
            }
           }
        #endif

        joy_xgain= Parameter( "JoyXgain" );
        joy_ygain= Parameter( "JoyYgain" );
        joy_zgain= Parameter( "JoyZgain" );
        XOffset= Parameter( "XOffset" );
        YOffset= Parameter( "YOffset" );
        ZOffset= Parameter( "ZOffset" );
/*shidong starts*/
        n_tmat= Ntargets;
        m_tmat= 10;              // was 4;   - now includes width and height  and adaptation code
                                // was 7;   - now includes 3rd dimension coordinate and depth
        for( i= 0; i<m_tmat; i++)               //9
        {
                for(j=0; j<n_tmat; j++)         //target numbers
                {
                        tmat[i][j]= Parameter("TargetPos",i,j);
                }
        }
/*shidong ends*/
        if( appl == NULL )
        {
                char FName[120];
                BCIDtry bcidtry;

                bcidtry.SetDir( FInit.c_str() );
                bcidtry.ProcPath();
                bcidtry.SetName( SName.c_str() );
                bcidtry.SetSession( SSes.c_str() );

                strcpy(FName, bcidtry.ProcSubDir() );

                strcat(FName,"\\");

                AName= SName + "S" + SSes + ".apl";
                strcat(FName, AName.c_str() );               // cpy vs cat

                if( (appl= fopen(FName,"a+")) != NULL)       // don't crash if NULL
                {
                  fprintf(appl,"%s \n",AName.c_str() );

                  ctime= time(NULL);
                  tblock= localtime(&ctime);
                  fprintf(appl,"%s \n", asctime( tblock ) );
                }
                else
                  bcierr << "Could not open " << FName << " for writing" << std::endl;
        }

        bitrate.Initialize(Ntargets);

        trial=1;
        slist=svect->GetStateListPtr();

        if (vis) delete vis;
        vis= new GenericVisualization;
        vis->SetSourceID(SOURCEID_TASKLOG);
        vis->SendCfg2Operator(SOURCEID_TASKLOG, CFGID_WINDOWTITLE, "User Task Log");
/*shidong starts*/
        User->Initialize( plist, slist, borderTexture, targetTexture, cursorTexture, Ntargets, TrackingTarget);
        User->setCursorColor( CursorColorFront, CursorColorBack);
        User->setCursor(CursorStartX, CursorStartY, CursorStartZ, CursorRadius, 0,1,0,255, cursorTexture);
        User->setTarget(tmat, Ntargets, m_tmat, targetTexture, borderTexture, WorkspaceBoundaryVisible);
        User->open3D();
        User->setWindow(WinWidth, WinHeight, WinXpos, WinYpos, WindowFullScreen, DisplayMonitor, ChangeResolution);
        User->setCameraLight(CameraX, CameraY, CameraZ, CameraAimX, CameraAimY, CameraAimZ, LightSourceX, LightSourceY, LightSourceZ, LightSourceColorR, LightSourceColorG, LightSourceColorB, LightSourceIntensity);

/*shidong ends*/
        User->GetLimits( &limit_right, &limit_left, &limit_top, &limit_bottom );
        User->GetSize( &size_right, &size_left, &size_top, &size_bottom );
        User->Scale( (float)0x7fff, (float)0x7fff );
        ComputeTargets( Ntargets );
        targetcount= 0;
        ranflag= 0;

        cursor_x_start= ( limit_right - limit_left ) * CursorStartX/100.0;
        cursor_y_start= ( limit_bottom - limit_top ) * CursorStartY/100.0;
if(printFlow) fprintf(b, "cursor start position are %f and %f.\n", cursor_x_start, cursor_y_start);

        time( &ctime );
        randseed= -ctime;

        ReadStateValues( svect );

        CurrentTarget= 0;
        TargetTime= 0;
        CurrentBaseline= 0;
        BaselineTime= 0;
        CurrentFeedback= 0;
        FeedbackTime= 0;
        CurrentIti= 1;
        ItiTime= 0;
        CurrentFeedback= 0;
        CurrentOutcome= 0;
        OutcomeTime= 0;
        CurrentRest= Resting;
        CurRunFlag= 0;
        Hits= 0;
        Misses= 0;

        PtpTime= 0;

        ntrials= 0;
        ntimeouts= 0;
        totalfeedbacktime= 0;
        feedflag= 0;

        CurrentXadapt= 0;
        CurrentYadapt= 0;
        CurrentZadapt= 0;
        CurrentAdaptCode= 0;

        User->PutO(false);

        #ifdef DATAGLOVE
        // if we want glove input, start streaming values from the joystick
        if (useJoy == 3)
           {
           delete my_glove;
           my_glove=new DataGlove();            // have to create it new; once stopped streaming, thread is terminated
           my_glove->StartStreaming(COMport);
           }
        #endif

        WriteStateValues( svect );
}

void TTask::ReadStateValues(STATEVECTOR *statevector)
{
/*shidong starts*/
if(printFlow) fprintf(b, "In TTask::ReadStateValues function.\n");
/*shidong ends*/
        CurrentTarget=       statevector->GetStateValue("TargetCode");
        CurrentOutcome=      statevector->GetStateValue("ResultCode");
        CurrentStimulusTime= statevector->GetStateValue("StimulusTime");
        CurrentFeedback=     statevector->GetStateValue("Feedback");
        CurrentIti=          statevector->GetStateValue("IntertrialInterval");
        CurrentRunning=      statevector->GetStateValue("Running");
                if( CurRunFlag == 1 )     // 0 must cycle through at least once
                {
                        if( CurrentRunning == 1 ) CurrentRunning = 0;
                        else                      CurRunFlag= 0;
                }

        CurrentRest=         statevector->GetStateValue("RestPeriod");

        CurrentXadapt=       statevector->GetStateValue("Xadapt");
        CurrentYadapt=       statevector->GetStateValue("Yadapt");
        CurrentZadapt=       statevector->GetStateValue("Zadapt");
        CurrentAdaptCode=    statevector->GetStateValue("AdaptCode");

}

void TTask::WriteStateValues(STATEVECTOR *statevector)
{
/*shidong starts*/
if(printFlow) fprintf(b, "In TTask::WriteStateValues function.\n\n");
/*shidong ends*/
        bcitime=new BCITIME;
        CurrentStimulusTime= bcitime->GetBCItime_ms();                   // time stamp
        delete bcitime;
        statevector->SetStateValue("StimulusTime",CurrentStimulusTime);

        statevector->SetStateValue("TargetCode",CurrentTarget);
        statevector->SetStateValue("ResultCode",CurrentOutcome);
        statevector->SetStateValue("ResponseTime",FeedbackTime);
        statevector->SetStateValue("Feedback",CurrentFeedback);
        statevector->SetStateValue("IntertrialInterval",CurrentIti);
        statevector->SetStateValue("Running",CurrentRunning);
        statevector->SetStateValue("RestPeriod",CurrentRest);
        /*shidong starts*/
        statevector->SetStateValue("CursorPosX",(int) (x_pos/ User->scalex) );
        statevector->SetStateValue("CursorPosY",(int) (y_pos/ User->scaley) );
        statevector->SetStateValue("CursorPosZ",(int) User->posZ/75*0x7fff );
        /*shidong ends*/
        statevector->SetStateValue("TrkCursorPosX", (int) (x_trkpos/ User->scalex) );
        statevector->SetStateValue("TrkCursorPosY", (int) (y_trkpos/ User->scaley) );
        statevector->SetStateValue("TrkCursorPosZ", (int) 0 );
        statevector->SetStateValue("Xadapt",CurrentXadapt);
        statevector->SetStateValue("Yadapt",CurrentYadapt);
        statevector->SetStateValue("Zadapt",CurrentZadapt);
        statevector->SetStateValue("AdaptCode",CurrentAdaptCode);

}

void TTask::ComputeTargets( int ntargs )
{
/*shidong starts*/
if(printFlow) fprintf(b, "In TTask::ComputeTargets function.\n");
/*shidong ends*/
        int i;
        float y_range;
        float x_range;

        y_range= size_bottom - size_top;
      //  y_range= y_range- (1.0 * TargetWidth);

        x_range= size_right - size_left;
     //   x_range= x_range - (1.0 * TargetHeight);
        /*shidong starts*/
        for(i=0;i<ntargs;i++)
        {
                targx[i+1]=          tmat[0][i] * x_range / 100;
                targsizex[i+1]=      tmat[3][i] * x_range / 100;
                targx_rt[i+1]=       targx[i+1] + targsizex[i+1];
                targy[i+1]=          tmat[1][i] * y_range / 100;
                targsizey[i+1]=      tmat[4][i] * y_range / 100;
                targy_btm[i+1]=      targy[i+1] + targsizey[i+1];
                targx_adapt[i+1]=    tmat[6][i];
                targy_adapt[i+1]=    tmat[7][i];
                targz_adapt[i+1]=    tmat[8][i];
                targ_adaptcode[i+1]= tmat[9][i];
        }
        /*shidong ends*/

}

void TTask::ShuffleTargs( int ntargs )
{
/*shidong starts*/
if(printFlow) fprintf(b, "In TTask::ShufflesTargs function.\n");
/*shidong ends*/
        int i,j;
        float rval;

        for(i=0;i<ntargs;i++)
        {
                rpt:    rval= User->ran1( &randseed );
                        targs[i]= 1 + (int)( rval * ntargs );

                if( (targs[i] < 1) || (targs[i]>Ntargets) )
                        goto rpt;

                for(j=0;j<i; j++)
                         if( targs[j]==targs[i] ) goto rpt;
        }
}

int TTask::GetTargetNo( int ntargs )
{
/*shidong starts*/
if(printFlow) fprintf(b, "In TTask::GetTargetNo function.\n");
/*shidong ends*/
        int retval;

        if( ranflag == 0 )
        {
                ShuffleTargs( ntargs );
                ranflag= 1;
        }

        retval= targs[targetcount];

        targetcount++;
        if( targetcount > (ntargs-1) )
        {
                ranflag= 0;
                targetcount= 0;
        }

        return( retval );

}

int TTask::TestTarget( float xpos, float ypos, int targ )
{
/*shidong starts*/
if(printFlow) fprintf(b, "In TTask::TestTarget function.\n");
/*shidong ends*/
        int result;
        int half;

        half= User->HalfCursorSize;

        result= 0;

        // could use cursor center by adding 1/2 of size to position
        /*shidong starts*/
       /*

        if( ( (xpos+half) > targx[targ] ) && ( (xpos-half) < targx_rt[targ] ) )
        {
                if( ( (ypos+half) > targy[targ] ) && ( (ypos-half) < targy_btm[targ] ) ) result= 1;
        }   */          //D2Box TestTarget

        //D3Box TestTarget
        if(collide(1,0, 2, targ))
        {
                result = 1;
                if(printFlow) fprintf(b, "Collision testing, current target is %d.\n", targ);
                if(sphereVec.size() != 0)
                {
                if(printFlow) fprintf(b, "Collision testing, sphere's paramters are: %f, %f, %f, %f.\n", sphereVec[0].getSphereX(), sphereVec[0].getSphereY(),sphereVec[0].getSphereZ(), sphereVec[0].getSphereRad());
                }
                else
                {
                        if(printFlow) fprintf(b, "Sphere size is %d.\n", sphereVec.size());
                }
                if(cuboidsVec.size() != 0)
                {
                        if(printFlow) fprintf(b, "Collision testing, Cuboid's paramters are: cube %d, %f, %f, %f, %f, %f, %f.\n", targ, cuboidsVec[targ].getCuboidX(), cuboidsVec[targ].getCuboidY(), cuboidsVec[targ].getCuboidZ(), cuboidsVec[targ].getCuboidW(), cuboidsVec[targ].getCuboidH(), cuboidsVec[targ].getCuboidD());
                }
                else
                {
                        if(printFlow) fprintf(b, "Cuboids size is %d.\n", cuboidsVec.size());
                }
        }
        /*shidong ends*/
        return( result );
}


void TTask::TestCursorLocation( float x, float y )
{
/*shidong starts*/
if(printFlow) fprintf(b, "In TTask::TestCursorLocation function.\n");
/*shidong ends*/
        int i;
        int res;

 //       x*= User->scalex;
 //       y*= User->scaley;


        if( targetInclude == 0 )
        {
                res= TestTarget( x, y, CurrentTarget );
                if( res == 1 )
                {
                        User->PutCursor( &x_pos, &y_pos, clBlue );
                        if (!TrackingTarget)
                           User->PutTarget( targx[ CurrentTarget ],
                                 targy[ CurrentTarget ],
                                 targsizex[ CurrentTarget ],
                                 targsizey[ CurrentTarget ],
                                 clYellow,
                                 CurrentTarget    );
                        CurrentFeedback= 0;
                        CurrentOutcome= CurrentTarget;
                        if( BaselineInterval == 1 )
                                CurrentBaseline= 0;
                }
        }


        else if( targetInclude == 1 )
        {

                res= TestTarget( x, y, CurrentTarget );         // test correct first
                if ( res == 1 )
                {
                        CurrentOutcome= CurrentTarget;
                        goto jmp;
                }

                for(i=1; i<Ntargets+1; i++)
                {
                        if( TestTarget( x, y, i ) == 1 )
                        {
                                CurrentOutcome= i;
                                goto jmp;
                        }
                }

                return;

jmp:            if( CurrentOutcome == CurrentTarget )
                {
                         if (!TrackingTarget)
                            User->PutTarget( targx[ CurrentTarget ],
                                 targy[ CurrentTarget ],
                                 targsizex[ CurrentTarget ],
                                 targsizey[ CurrentTarget ],
                                 clYellow,
                                 CurrentTarget    );
                          Hits++;
                          HitOrMiss=true;
                }
                else
                {
                         if (!TrackingTarget)
                            User->PutTarget( targx[ CurrentTarget ],
                                 targy[ CurrentTarget ],
                                 targsizex[ CurrentTarget ],
                                 targsizey[ CurrentTarget ],
                                 clBlack,
                                 CurrentTarget    );
                         Misses++;
                         HitOrMiss=false;
                }
                CurrentFeedback= 0;
                if( BaselineInterval == 1 )
                        CurrentBaseline= 0;
        }

}

void TTask::UpdateSummary( void )
{
/*shidong starts*/
if(printFlow) fprintf(b, "In TTask::UpdateSummary function.\n");
/*shidong ends*/
        float pc;

        fprintf(appl," @ D3Box @   Number of Targets=%2d \n",Ntargets);

        if( targetInclude == 0 )
        {
                fprintf(appl,"NoError Mode Run %2d  Trials  %3d  Timeouts  %3d \n",run,ntrials,ntimeouts);
                if( ntrials > 0 ) fprintf(appl,"   Mean trial duration= %6.2f \n",totalfeedbacktime/(double)(ntrials) );
        }
        else
        {

                if( Hits+Misses > 0 )    pc= 100.0 * Hits / (Hits+Misses);
                else                     pc= -1.0;

                fprintf(appl,"Run %2d   Hits=%3d  Total=%3d  Percent=%6.2f \n",run,Hits,Hits+Misses,pc);
                fprintf(appl,"Bits= %7.2f \n",bitrate.TotalBitsTransferred() );
        }



        fprintf(appl,"Time Passed (sec)=%7.2f \n",timepassed);
        fprintf(appl,"..........................................................\n\n");
}

void TTask::UpdateDisplays( void )
{
/*shidong starts*/
if(printFlow) fprintf(b, "In TTask::UpdateDisplays function.\n");
/*shidong ends*/

char            memotext[256];


 if (CurrentRunning == 0)
    CurrentRunning=0;

 if ((CurrentRunning == 0) && (OldRunning == 1))            // put the T up there on the transition from not suspended to suspended
    {
    User->Clear();
    User->PutT(true);
    CurrentIti=1;
    }
 if (CurrentRunning == 1)
    {
    if (OldRunning == 0)
       {
       runstarttime=(double)(TDateTime::CurrentTime());   // run just started ?
       run++;
       }
    User->PutT(false);
    // in the ITI period ?
    if (CurrentIti > 0)
       {
       // ITI period just started
       if (ItiTime == 0)
          {
          sprintf(memotext, "Run: %d; ITI -> new trial: %d\r", run, trial);
          vis->SendMemo2Operator(memotext);
          trial++;
          }
            // in case the run was longer than x seconds
            timepassed= ((double)(TDateTime::CurrentTime())-runstarttime)*86400;

            if (timepassed > timelimit)
               {
               CurrentRunning=0;
               CurRunFlag= 1;
               if (Hits+Misses > 0)
                  {
                  sprintf(memotext, "Run %d - %.1f%% correct\r", run, (float)Hits*100/((float)Hits+(float)Misses));
                  vis->SendMemo2Operator(memotext);
              //    fprintf( appl,"%s \n",memotext);
                  }
               else
                  {
                  vis->SendMemo2Operator("No statistics for this run\r");
                  vis->SendMemo2Operator("There were no trials\r");
                  }
               sprintf(memotext, "Total Bits transferred: %.2f\r", bitrate.TotalBitsTransferred());
               vis->SendMemo2Operator(memotext);
            //   fprintf( appl,"%s \n",memotext);
               sprintf(memotext, "Average Bits/Trial: %.2f\r", bitrate.BitsPerTrial());
               vis->SendMemo2Operator(memotext);
           //    fprintf( appl,"%s \n",memotext);
               sprintf(memotext, "Average Bits/Minute: %.2f\r", bitrate.BitsPerMinute());
               vis->SendMemo2Operator(memotext);
            //   fprintf( appl,"%s \n",memotext);
               vis->SendMemo2Operator("**************************\r");
             //  fprintf( appl,"**************************\r\n");
               User->Clear();
               User->PutT(true);
               UpdateSummary();
               }
        }

    else if (CurrentOutcome > 0 )
            {
            // trial just ended ...
            if (OutcomeTime == 0)
               {
               sprintf(memotext, "%d hits %d missed\r", Hits, Misses);
               vis->SendMemo2Operator(memotext);
               bitrate.Push(HitOrMiss);
               }

            }
    }

 // new trial just started
 if ((CurrentTarget > 0) && (OldCurrentTarget == 0))
    {
    sprintf(memotext, "Target: %d\r", CurrentTarget);
    vis->SendMemo2Operator(memotext);
    }

 OldRunning=CurrentRunning;
 OldCurrentTarget=CurrentTarget;
}

void TTask::Iti( void )
{
/*shidong starts*/
if(printFlow) fprintf(b, "In TTask::Iti function.\n");
/*shidong ends*/
        if( BaselineInterval == 2 )
                CurrentBaseline= 1;
        User->Clear();
        ItiTime++;

        if( ItiTime > ItiDuration )
        {
                FeedbackTime= 0;
                CurrentIti = 0;
                ItiTime = 0;
                CurrentTarget= GetTargetNo( Ntargets );

                if (!TrackingTarget)
                   User->PutTarget( targx[ CurrentTarget ],
                                 targy[ CurrentTarget ],
                                 targsizex[ CurrentTarget ],
                                 targsizey[ CurrentTarget ],
                                 clRed,
                                 CurrentTarget    );

                if( BaselineInterval == 1 )
                        CurrentBaseline= 1;
                if( BaselineInterval == 2 )
                        CurrentBaseline= 0;


                CurrentXadapt= targx_adapt[CurrentTarget];
                CurrentYadapt= targy_adapt[CurrentTarget];
                CurrentZadapt= targz_adapt[CurrentTarget];
                CurrentAdaptCode= targ_adaptcode[CurrentTarget];
         }
         if( (targetInclude == 0) && (feedflag == 1) )
         {
                totalfeedbacktime+= ((double)(TDateTime::CurrentTime() ) - feedstart ) * 86400;
                feedflag= 0;
         }
}

void TTask::Ptp( void )
{
/*shidong starts*/
if(printFlow) fprintf(b, "In TTask::Ptp function.\n");
if(printFlow) fprintf(b, "PtpTime is %d.\n", PtpTime);
/*shidong ends*/
        MMRESULT mmres;
        JOYINFO joyi;

        PtpTime++;

        if( PtpTime > PtpDuration )
        {

                x_pos= cursor_x_start;
                y_pos= cursor_y_start;
                x_trkpos= cursor_x_start;
                y_trkpos= cursor_y_start;
                x_radius=ellipse_radius_x;
                y_radius=ellipse_radius_y;
                cntr=0;
                
if(printFlow) fprintf(b, "In Ptp x and y pos are: %f, %f.\n", x_pos, y_pos);
                User->PutCursor( &x_pos, &y_pos, clRed );
                CurrentFeedback= 1;
                FeedbackTime= 0;

                PtpTime= 0;

                if( useJoy == 1 )   // try && (PtpTime == 1) )
                {
                         // center the joystick based on the current joystick position
                         // mmres= joyGetPos( JOYSTICKID1, &joyi );
                         // jx_cntr= joyi.wXpos-32768;
                         // jy_cntr= joyi.wYpos-32768;
                         // center the joystick based on user-defined parameters;
                         jx_cntr=(int)XOffset;
                         jy_cntr=(int)YOffset;
                }
        }

}


void TTask::GetJoy( )
{
/*shidong starts*/
if(printFlow) fprintf(b, "In TTask::GetJoy function.\n");
/*shidong ends*/
        MMRESULT mmres;
        JOYINFO joyi;
        float tx;
        float ty;

        mmres= joyGetPos( JOYSTICKID1, &joyi );
        //shidong starts   JOYSTICK CONTROL 3rd Dimension
        // turned off for now
        /*
        if (joyi.wButtons == JOY_BUTTON1)
                User->posZ += 0.2;
        if (joyi.wButtons == JOY_BUTTON2)
                User->posZ -= 0.2;
        */
if(printFlow) fprintf(b, "In TTask::GetJoy function, Zoffset is %f and joy_zgain is %f, posZ is %f.\n",ZOffset, joy_zgain, User->posZ);
        User->posZ = ((User->posZ)-(float)(int)ZOffset)*joy_zgain ;
if(printFlow) fprintf(b, "In TTask::GetJoy function, Zoffset is %f and joy_zgain is %f, posZ is %f.\n",ZOffset, joy_zgain, User->posZ);
                        //make sure posZ is within Boundary
                        if (User->posZ > 0) User->posZ = 0;
                        if (User->posZ < (0-BORDER)) User->posZ = (0-BORDER);

        if(sphereVec.size() != 0)
{
sphereVec[0].setSphere(sphereVec[0].getSphereX(), sphereVec[0].getSphereY(), User->posZ, sphereVec[0].getSphereRad() );
}

        //shidong ends

        tx= (float)(joyi.wXpos)-32768  - (float)(jx_cntr); //jx_cntr is xoffset
        ty= (float)(joyi.wYpos)-32768  - (float)(jy_cntr);

        tx= tx / (float)jhalf;  //jhalf is 32767
        ty= ty / (float)jhalf;

        x_pos+= tx * joy_xgain;
        y_pos+= ty * joy_ygain;
}


void TTask::GetMouse( )
{
// x_pos and y_pos are in pixels (i.e., the same as window size)

  x_pos= 512*((float)Mouse->CursorPos.x)/((float)Screen->Width) * joy_xgain;
  y_pos= 512*((float)Mouse->CursorPos.y)/((float)Screen->Height) * joy_ygain;
}


#ifdef DATAGLOVE
void TTask::GetGlove( )
{
/*shidong starts*/
if(printFlow) fprintf(b, "In TTask::GetGlove function.\n");
/*shidong ends*/
float cur_glovevalx, cur_glovevaly;

        cur_glovevalx=0;
        cur_glovevaly=0;
        // we are doing the following transformation:
        // deltax=sum(abs(sensor(t)*weight1(t)+sensor(t-1)*weight2(t-1))*sign) with sign determined by the parameter's third row
        for (int i=0; i<MAX_GLOVESENSORS; i++)
            {
            float old_sensorval=(float)my_glove->GetSensorValOld(i);
            float cur_sensorval=(float)my_glove->GetSensorVal(i);
            float cur_signx, cur_signy;
            if (GloveControlX[2][i] >= 0) cur_signx=1;
            else cur_signx=-1;
            if (GloveControlY[2][i] >= 0) cur_signy=1;
            else cur_signy=-1;
            cur_glovevalx += fabs(GloveControlX[0][i]*old_sensorval+GloveControlX[1][i]*cur_sensorval)*cur_signx;
            cur_glovevaly += fabs(GloveControlY[0][i]*old_sensorval+GloveControlY[1][i]*cur_sensorval)*cur_signy;
            AnsiString glovestatename="GloveSensor"+AnsiString(i+1);
            State(glovestatename.c_str())=(unsigned short)my_glove->GetSensorVal(i);
            }
        // my_glove->ReadSensorsFromGlove("COM2");
        x_pos+= (cur_glovevalx-XOffset) * joy_xgain;
        y_pos+= (cur_glovevaly-YOffset) * joy_ygain;
        if (my_glove->GetGloveErr() != GLOVE_ERR_NOERR)
           Application->MessageBox("Glove Error", MB_OK);
}
#endif

void TTask::Feedback( short sig_x, short sig_y )
{
/*shidong starts*/
if(printFlow) fprintf(b, "In TTask::Feedback function.\n");
if(printFlow) fprintf(b, "FeedbackTime is %d.\n", FeedbackTime);
if(printFlow) fprintf(b, "FeedbackTime Signals are %hd and %hd.\n", sig_x, sig_y);
if(printFlow) fprintf(b, "Starting position are %f and %f.\n", x_pos, y_pos);
if(printFlow) fprintf(b, "scalex and y are %f and %f.\n", User->scalex, User->scaley);
/*shidong ends*/
        if(useJoy == 0)
        {
                x_pos+= (float)sig_x * User->scalex;
                y_pos+= (float)sig_y * User->scaley;
                if(printFlow) fprintf(b, "USEJOY == 0.\n");
        }
        if(useJoy == 1) GetJoy();
        if(useJoy == 2) GetMouse();
        #ifdef DATAGLOVE
        if(useJoy == 3) GetGlove();
        #endif
if(printFlow) fprintf(b, "After mult scale Signals are %f and %f.\n", x_pos, y_pos);
        User->PutCursor( &x_pos, &y_pos, clRed );
        if (TrackingTarget)
           {
           // triangle
           if (tracking_shape == 2)
              {
              if (cntr <= 0.33)
                 {
                 x_trkpos-=0.7071*triangle_size_x;
                 y_trkpos+=1.41*triangle_size_y;
                 }
              else
                 {
                 if (cntr <= 0.66)
                    {
                    x_trkpos+=1.5*triangle_size_x;
                    y_trkpos+=0;
                    }
                 else
                    {
                    x_trkpos-=0.7071*triangle_size_x;
                    y_trkpos-=1.41*triangle_size_y;
                    }
                 }
              }
           // ellipse
           if (tracking_shape == 1)
              {
              x_radius-=cntr*ellipse_radius_decrement_x;
              y_radius-=cntr*ellipse_radius_decrement_y;
              x_trkpos=(16384+x_radius*cos(cntr))*User->scalex;
              y_trkpos=(16384+y_radius*sin(cntr))*User->scaley;
              }
           User->PutTrackingTarget( &x_trkpos, &y_trkpos, clRed );

           cntr+=tracking_speed;
           if (tracking_shape == 2)  // reset triangle if back at top
              if (cntr > 1) cntr=0;
           }
if(printFlow) fprintf(b, "After put cursor are %f and %f.\n", x_pos, y_pos);
        TestCursorLocation( x_pos, y_pos );
if(printFlow) fprintf(b, "After TestCursorLocation are %f and %f.\n", x_pos, y_pos);

        FeedbackTime++;

        if( FeedbackTime > FeedbackDuration )
        {
                //   FeedbackTime= 0;
                CurrentFeedback= 0;
                CurrentOutcome= 0;
                CurrentIti= 1;
                ntimeouts++;
        }
        if( feedflag == 0 )
        {
                feedflag= 1;
                ntrials++;
                feedstart= (double)(TDateTime::CurrentTime());
        }
}


void TTask::Outcome()
{
/*shidong starts*/
if(printFlow) fprintf(b, "In TTask::Outcome function.\n");
/*shidong ends*/
        OutcomeTime++;

        if( OutcomeTime > OutcomeDuration )
        {
                CurrentIti= 1;
                CurrentOutcome= 0;
                CurrentTarget= 0;
                OutcomeTime= 0;
                User->PutCursor( &x_pos, &y_pos, clBlack );
                if (TrackingTarget)
                   User->PutTrackingTarget( &x_pos, &y_pos, clBlack );
        }

        if( (targetInclude == 0) && (feedflag == 1) )
        {
                totalfeedbacktime+= ((double)(TDateTime::CurrentTime() ) - feedstart ) * 86400;
                feedflag= 0;
        }
}

void TTask::Rest( void )
{
/*shidong starts*/
if(printFlow) fprintf(b, "In TTask::Rest function.\n");
/*shidong ends*/
        if ((CurrentRunning == 1) && (OldRunning == 0))
        {
                runstarttime=(double)(TDateTime::CurrentTime());   // run just started ?
                run++;
                OldRunning= 1;
                User->Clear();
                User->PutT(false);
                User->PutO(true);
               // CurrentRest= 1;
                CurrentIti= 0;

        }

        timepassed= ((double)(TDateTime::CurrentTime())-runstarttime)*86400;

         if (timepassed > timelimit)
         {
               CurrentRunning=0;
               User->Clear();
               User->PutO(false);
               User->PutT(true);
               CurrentRest= 0;
               CurrentIti= 1;
         }

}

void TTask::Process( const GenericSignal * Input, GenericSignal * Output )
{
/*shidong starts*/
if(printFlow) fprintf(b, "In TTask::Process function.\n");
/*shidong ends*/
        ReadStateValues( Statevector );

        if( CurrentRunning > 0 )
        {
                if(CurrentRest > 0 )             Rest();
                else if (CurrentIti > 0)         Iti();
                else if (CurrentFeedback > 0 )   Feedback( ( *Input )( 1, 0 ), ( *Input )( 0, 0 ) );
                else if (CurrentOutcome  > 0 )   Outcome();
                else if (CurrentTarget   > 0 )   Ptp();
        }

/*shidong starts*/
if(printFlow) fprintf(b, "Running is %d, Rest is %d, Iti is %d, CurrentFeedback is %d, Outcome is %d, Target is %d.\n", CurrentRunning, CurrentRest, CurrentIti, CurrentFeedback, CurrentOutcome, CurrentTarget);
if(printFlow) fprintf(b, "The input singal is %f, and %f.\n", ( *Input )( 1, 0 ), ( *Input )( 0, 0 ));
/*shidong ends*/
        UpdateDisplays();
        WriteStateValues( Statevector );
        *Output = *Input;
}


