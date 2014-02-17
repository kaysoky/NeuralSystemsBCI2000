#include "PCHIncludes.h"
#pragma hdrstop

#include "B2B.h"
#include "Localization.h"
#include "DFBuildScene2D.h"

#include "buffers.h"
#include <stdio.h>
#include <math.h>
#include <sstream>
#include <algorithm>

#include <QImage>

// State variable for initialization and other usage(s)
#define CURSOR_POS_BITS "12"
const int cCursorPosBits = ::atoi( CURSOR_POS_BITS );

// Initial values of several state variables
#define SCORE_BITS "16"
#define TARGET_XWIDTH_BITS "12"
#define TARGET_YWIDTH_BITS "12"
#define CURSOR_RADIUS_BITS "12"
#define PRIMARY_AXIS_BITS "12"

// Used to pass state between this class (C++) and Mongoose (C)
static DynamicFeedbackTask *currentTask;
static void *CountdownServerThread(void *arg);
static int CountdownServerHandler(struct mg_connection *conn);

RegisterFilter( DynamicFeedbackTask, 3 );

using namespace std;

DynamicFeedbackTask::DynamicFeedbackTask()
    : mpFeedbackScene( NULL ),
      mpMessage( NULL ),
      mpMessage2( NULL ),
      mCursorColor( RGBColor::White ),
      mRunCount( 0 ),
      mTrialCount( 0 ),
      mCurFeedbackDuration( 0 ),
      mMaxFeedbackDuration( 0 ),
      mCursorSpeedX( 1.0 ),
      mCursorSpeedY( 1.0 ),
      mCursorSpeedZ( 1.0 ),
      mScore(0.0),
      mScoreCount(0.0),
      mrWindow( Window() ),
      mVisualFeedback( false),
      mIsVisualCatchTrial( false ) {
    BEGIN_PARAMETER_DEFINITIONS
        "Application:Targets matrix Targets= "
        " 2 " // rows
        " [pos%20x pos%20y width%20x width%20y] " // columns
        " 50 90 8 8 "
        " 50 10 8 8 "
        " // Number of targets and their position (center) and dimensions in percentage coordinates",
    "Application:Targets int TargetColor= 0x0000FF % % % " // Blue
        " // target color (color)",

    "Application:Cursor float CursorWidth= 10 10 0.0 % "
        " // feedback cursor width in percent of screen width",
    "Application:Cursor float CursorSpeedMult= 3"
        " // Cursor speed from to 1(slowest) to 10(fastest) ",
    "Application:Cursor float CursorGravity= 1"
        " // Centering cursor gravity from to 0 (weakest) to 10 (strongest) ",
    "Application:Cursor int CursorColor= 0xff0000 % % % " // Red
        " // Cursor color (color)",
    "Application:Cursor floatlist CursorPos= 3 50 50 50 % % "
        " // cursor starting position",

    "Application:Sequencing float MaxFeedbackDuration= 3s % 0 % "
        " // abort a trial after this amount of feedback time has expired",

    "Application:3DEnvironment int WorkspaceBoundaryColor= 0xffffff 0 % % "
        " // workspace boundary color (0xff000000 for invisible) (color)",
    "Application:Feedback int VisualFeedback= 1 1 0 1 "
        "// provide visual stimulus (boolean)",
    "Application:Feedback intlist VisualCatchTrials= 4 1 3 4 2 % % % // "
        "// list of visual catch trials, leave empty for none",

    "Application:Connector string ListeningPort= % "
    "20320 % % // Port for the server to listen on",
    END_PARAMETER_DEFINITIONS

    BEGIN_STATE_DEFINITIONS
        "CursorPosX " CURSOR_POS_BITS " 0 0 0",
        "CursorPosY " CURSOR_POS_BITS " 0 0 0",
        "GameScore " SCORE_BITS " 0 0 0", //score bits
        "TargetXWidth " TARGET_XWIDTH_BITS " 0 0 0", //target width in x
        "TargetYWidth " TARGET_YWIDTH_BITS " 0 0 0", //target width in y
        "CursorRadius " CURSOR_RADIUS_BITS " 0 0 0", //cursor radius
    END_STATE_DEFINITIONS

    // Title screen message
    GUI::Rect rect = {0.5f, 0.4f, 0.5f, 0.6f};
    mpMessage = new TextField(mrWindow);
    mpMessage->SetTextColor(RGBColor::Lime)
              .SetTextHeight(0.8f)
              .SetColor(RGBColor::Gray)
              .SetAspectRatioMode(GUI::AspectRatioModes::AdjustWidth)
              .SetObjectRect(rect);

    // Score message in top right corner
    GUI::Rect rect3 = {0.89f, 0.01f, .99f, 0.09f};
    mpMessage2 = new TextField(mrWindow);
    mpMessage2->SetTextColor(RGBColor::Black)
               .SetTextHeight(0.45f)
               .SetColor(RGBColor::White)
               .SetAspectRatioMode(GUI::AspectRatioModes::AdjustNone)
               .SetObjectRect(rect3);
    
    // Synchronization for the countdown server
    server_lock = new OSMutex();
    
    // A new server instance is created every time the configuration is set
    server = NULL;
}

DynamicFeedbackTask::~DynamicFeedbackTask() {
    delete mpFeedbackScene;
}

void
DynamicFeedbackTask::OnPreflight(const SignalProperties& Input) const {
    if (Parameter("Targets")->NumValues() <= 0) {
        bcierr << "At least one target must be specified" << endl;
    }

    if (Parameter("CursorPos")->NumValues() != 3) {
        bcierr << "Parameter \"CursorPos\" must have 3 entries" << endl;
    }

    const char* colorParams[] = {
        "CursorColor",
        "TargetColor",
        "WorkspaceBoundaryColor"
    };
    for (size_t i = 0; i < sizeof(colorParams) / sizeof(*colorParams); ++i) {
        if (RGBColor(Parameter(colorParams[i])) == RGBColor(RGBColor::NullColor)) {
            bcierr << "Invalid RGB value in " << colorParams[ i ] << endl;
        }
    }

    if (Parameter("FeedbackDuration").InSampleBlocks() <= 0) {
        bcierr << "FeedbackDuration must be greater 0" << endl;
    }

    if (Parameter("CursorSpeedMult") > 10 || Parameter("CursorSpeedMult") < 1) {
        bcierr << "Cursor speed must be between 1 and 10" << endl;
    }

    if (Parameter("CursorGravity") > 10 ||  Parameter("CursorGravity") < 0) {
        bcierr << "Cursor gravity must be between 1 and 10" << endl;
    }

    Parameter("SampleBlockSize");
    Parameter("ListeningPort");

    if (Parameter("VisualFeedback") == 1) {
        ParamRef visualCatch = Parameter("VisualCatchTrials");
        for (int i = 0; i < visualCatch->NumValues(); ++i) {
            if (visualCatch(i) < 1) {
                bcierr << "Invalid stimulus code "
                       << "(" << visualCatch(i) << ") "
                       << "at visualCatch(" << i << ")"
                       << endl;
            }
        }
    }
}

void
DynamicFeedbackTask::OnInitialize(const SignalProperties& Input) {
  // Give C-functions a reference to this program
  currentTask = this;

	// Determine where this application is being executed
	char buffer[MAX_PATH];
    GetModuleFileName(NULL, buffer, MAX_PATH);
    string::size_type pos = string(buffer).find_last_of( "\\/" );
	string gameLocation = string(buffer).substr(0, pos);
	gameLocation += "/CountdownGame/";

    // Initialize the server
    server_lock->Acquire();
    if (server != NULL) {
        mg_destroy_server(&server);
    }
	string listeningPort = string(Parameter("ListeningPort"));
	server = mg_create_server(NULL);
	mg_set_option(server, "document_root", gameLocation.c_str());
    mg_set_option(server, "listening_port", listeningPort.c_str());
    mg_set_request_handler(server, CountdownServerHandler);

    // Start the server on a separate thread
    lastClientPost = CONTINUE;
    targetHit = false;
    mg_start_thread(CountdownServerThread, NULL);
	bciout << "Server listening on port " << mg_get_option(server, "listening_port");
  server_lock->Release();

    // Cursor speed in pixels per signal block duration:
    float feedbackDuration = Parameter("FeedbackDuration").InSampleBlocks();

    // On average, we need to cross half the workspace during a trial.
    mCursorSpeedX = 100.0 / feedbackDuration / 2;
    mCursorSpeedY = 100.0 / feedbackDuration / 2;
    mCursorSpeedZ = 100.0 / feedbackDuration / 2;

    mMaxFeedbackDuration = static_cast<int>(Parameter("MaxFeedbackDuration").InSampleBlocks());

    mCursorColor = RGBColor(Parameter("CursorColor"));

    delete mpFeedbackScene;
    mpFeedbackScene = new DFBuildScene2D(mrWindow);
    mpFeedbackScene->Initialize();
    mpFeedbackScene->SetCursorColor(mCursorColor);

    mrWindow.Show();
    DisplayMessage("Timeout");
    DisplayScore("0");

    mVisualFeedback = Parameter("VisualFeedback") == 1;

    if (mVisualFeedback == true) {
        mVisualCatchTrials.clear();
        for (int j = 0; j < Parameter("VisualCatchTrials")->NumValues(); ++j) {
            mVisualCatchTrials.push_back(Parameter("VisualCatchTrials")(j));
        }
    }
}

void
DynamicFeedbackTask::OnStartRun() {
    ++mRunCount;
    mTrialCount = 0;
    mTrialStatistics.Reset();
    mScore = 0;
    State("GameScore") = mScore;
    State("CursorRadius") = Parameter("CursorWidth");

    AppLog << "Run #" << mRunCount << " started" << endl;
    DisplayMessage(">> Get Ready! <<");
}

void
DynamicFeedbackTask::DoPreRun(const GenericSignal&, bool& doProgress) {
    // Wait for the start signal
    doProgress = false;
    
    server_lock->Acquire();
    if (lastClientPost == START_TRIAL) {
        doProgress = true;
        lastClientPost = CONTINUE;
    }
    server_lock->Release();
}

void
DynamicFeedbackTask::OnTrialBegin() {
    ++mTrialCount;
    AppLog.Screen << "Trial #" << mTrialCount
                  << ", target: " << State("TargetCode")
                  << endl;

    if (mVisualFeedback == true) {
        mIsVisualCatchTrial = false;
        for (size_t i = 0; i < mVisualCatchTrials.size(); i++) {
            mIsVisualCatchTrial = (mVisualCatchTrials.at(i) == mTrialCount);
        }

        if (mIsVisualCatchTrial == true) {
            AppLog.Screen << "<- visual catch trial" << endl;
        }
    }


    enum {x, y, dx, dy};
    ParamRef Targets = Parameter("Targets");
    State("TargetXWidth") = Targets(State("TargetCode") - 1, dx);
    State("TargetYWidth") = Targets(State("TargetCode") - 1, dy);

    DisplayMessage("");
    RGBColor targetColor = RGBColor(Parameter("TargetColor"));
    for (int i = 0; i < mpFeedbackScene->NumTargets(); ++i) {
        mpFeedbackScene->SetTargetColor(targetColor, i);
        mpFeedbackScene->SetTargetVisible(State("TargetCode") == (i + 1), i);
    }
    
    // Reset the 'targetHit' value
    server_lock->Acquire();
    targetHit = false;
    server_lock->Release();
}

void
DynamicFeedbackTask::OnFeedbackBegin() {
    mCurFeedbackDuration = 0;

    enum { x, y, z };
    ParamRef CursorPos = Parameter("CursorPos");
    MoveCursorTo(CursorPos(x), CursorPos(y), CursorPos(z));
    if (mVisualFeedback == true && mIsVisualCatchTrial == false) {
        mpFeedbackScene->SetCursorVisible(true);
    }
}

void
DynamicFeedbackTask::DoFeedback(const GenericSignal& ControlSignal, bool& doProgress) {
	doProgress = false;

    // Update cursor position
    float x = mpFeedbackScene->CursorXPosition(),
    y = mpFeedbackScene->CursorYPosition(),
    z = mpFeedbackScene->CursorZPosition();
	
	// Use the control signal to move up and down
	if (ControlSignal.Channels() > 0) {
		y += mCursorSpeedX * ControlSignal( 0, 0 );
	}

    // Restrict cursor movement to the inside of the bounding box:
    float r = mpFeedbackScene->CursorRadius();
    x = max(r, min(100 - r, x)),
    y = max(r, min(100 - r, y)),
    z = max(r, min(100 - r, z));
    mpFeedbackScene->SetCursorPosition(x, y, z);

    const float coordToState = ((1 << cCursorPosBits) - 1) / 100.0;
    State("CursorPosX") = static_cast<int>(x * coordToState);
    State("CursorPosY") = static_cast<int>(y * coordToState);

    if( mpFeedbackScene->TargetHit(State("TargetCode") - 1)) {
        State("ResultCode") = State("TargetCode");
        mpFeedbackScene->SetCursorColor(RGBColor::White);
        mpFeedbackScene->SetTargetColor(RGBColor::Red, State("ResultCode") - 1);

        doProgress = true;
        
        // Send message of hit out to the countdown game
        server_lock->Acquire();
        targetHit = true;
        server_lock->Release();
    }
    
    // Check for the stop signal
    // Note: This seemingly redundant double-check is necessary to prevent race conditions
    if (lastClientPost == STOP_TRIAL) {
        server_lock->Acquire();
        if (lastClientPost == STOP_TRIAL) {
            doProgress = true;
            lastClientPost = CONTINUE;
        }
        server_lock->Release();
    }
}

void
DynamicFeedbackTask::OnFeedbackEnd() {
    if (State("ResultCode") == 0) {
        AppLog.Screen << "-> aborted" << endl;
        mTrialStatistics.UpdateInvalid();

    } else {
        mTrialStatistics.Update(State("TargetCode"), State("ResultCode"));
        if (State("TargetCode") == State("ResultCode")) {
            AppLog.Screen << "-> hit\n " << "Your Score:" << mScore << endl;
            State("GameScore") = mScore;
        } else {
            mScore = mScore;
            AppLog.Screen << "-> miss\n " << "Your Score:" << mScore << endl;
            State("GameScore") = mScore;
        }
    }

    // Persistent Score Display
    stringstream ss (stringstream::in | stringstream::out);
    int intScore = mScore >= 0 ? (int)(mScore + 0.5) : (int)(mScore - 0.5);
    ss << intScore;
    DisplayScore(ss.str());
}

void
DynamicFeedbackTask::OnTrialEnd(void) { };

void
DynamicFeedbackTask::DoITI(const GenericSignal&, bool& doProgress) {
    // Wait for the start signal
    doProgress = false;
    
    server_lock->Acquire();
    if (lastClientPost == START_TRIAL) {
        doProgress = true;
        lastClientPost = CONTINUE;
    }
    server_lock->Release();
}

void
DynamicFeedbackTask::OnStopRun() {
    AppLog << "Run " << mRunCount        << " finished: "
           << mTrialStatistics.Total()   << " trials, "
           << mTrialStatistics.Hits()    << " hits, "
           << mTrialStatistics.Invalid() << " invalid.\n";
    int validTrials = mTrialStatistics.Total() - mTrialStatistics.Invalid();
    if (validTrials > 0)
    AppLog << (200 * mTrialStatistics.Hits() + 1) / validTrials / 2  << "% correct, "
           << mTrialStatistics.Bits() << " bits transferred.\n, "
           << "Game Score:\n " << mScore
           << "====================="  << endl;

    DisplayMessage("Timeout");
}


// Access to graphic objects
void
DynamicFeedbackTask::MoveCursorTo(float inX, float inY, float inZ) {
    mpFeedbackScene->SetCursorPosition(inX, inY, inZ);
}

void
DynamicFeedbackTask::DisplayMessage(const string& inMessage) {
    if (inMessage.empty()) {
        mpMessage->Hide();
    } else {
        mpMessage->SetText(" " + inMessage + " ");
        mpMessage->Show();
    }
}
void
DynamicFeedbackTask::DisplayScore(const string&inMessage) {
    if (inMessage.empty()) {
        mpMessage2->Hide();
    } else {
        mpMessage2->SetText("+" + inMessage + " ");
        mpMessage2->Show();
    }
}

/*
 * Loops infinitely and polls for incoming connections
 */
static void *CountdownServerThread(void *arg) {
    // Keep polling the server
    // Note: the server can be swapped out at any time (hence the locking)
    while (true) {
        currentTask->server_lock->Acquire();
        mg_poll_server(currentTask->server, 10);
        currentTask->server_lock->Release();
    }

    return NULL;
}

/* 
 * Handles the pre-defined set of REST methods
 * Other methods are passed along to Mongoose for default handling
 * Note: the calling thread already holds the lock when this request handler is called
 * The call stack is something like:
 *   CountdownServerThread
 *     -> mg_poll_server
 *     -> CountdownServerHandler
 */
static int CountdownServerHandler(struct mg_connection *conn) {
    string method(conn->request_method);
    string uri(conn->uri);
    bciout << method << " " << uri << endl;
    
    if (method.compare("POST") == 0) {
        // These methods set an internal variable for the other threads to monitor
        // Whenever the value is read at the appropriate stage of the trial, 
        //   'lastClientPost' is reset to CONTINUE
        
        if (uri.compare("/trial/start") == 0) {
            mg_send_status(conn, 204);
            mg_send_data(conn, "", 0);
            currentTask->lastClientPost = DynamicFeedbackTask::TrialState::START_TRIAL;
            
            return MG_REQUEST_PROCESSED;
            
        } else if (uri.compare("/trial/stop") == 0) {
            mg_send_status(conn, 204);
            mg_send_data(conn, "", 0);
            currentTask->lastClientPost = DynamicFeedbackTask::TrialState::STOP_TRIAL;
            
            return MG_REQUEST_PROCESSED;
        }
        
    } else if (method.compare("GET") == 0) {
        // This allows the Countdown game to poll for a hit
        // 1 == hit, 0 == no hit
        if (uri.compare("/trial/hit") == 0) {
            if (currentTask->targetHit) {
                mg_send_status(conn, 200);
                mg_send_header(conn, "Content-Type", "text/plain");
                mg_printf_data(conn, "HIT");
            } else {
              mg_send_status(conn, 204);
              mg_send_data(conn, "", 0);
            }
            
            return MG_REQUEST_PROCESSED;
        } 
    }
    
    return MG_REQUEST_NOT_PROCESSED;
}
