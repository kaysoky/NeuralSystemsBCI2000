#include "PCHIncludes.h"
#pragma hdrstop

#include "MongooseTask.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sstream>
#include <algorithm>
#include <cstdlib>

/*
 * The currently running task
 */
static MongooseTask *currentTask;
static void *MongooseServerThread(void *arg);

MongooseTask::MongooseTask()
    : nextTrialType(NULL) {
    
    BEGIN_PARAMETER_DEFINITIONS
    "Application:Mongoose string FileDirectory= % "
        "CountdownGame/ % % //"
    "Application:Mongoose string ListeningPort= % "
        "20320 % % // Port for the server to listen on",
    END_PARAMETER_DEFINITIONS

    BEGIN_STATE_DEFINITIONS
        "TrialType              2 0 0 0", // 0 for Airplane, 1 for Missile
        "CountdownHitReported   1 0 0 0", // Spacebar was pressed in the Countdown game
        "CountdownMissileScore  8 0 0 0", // Number of missiles shot
        "CountdownAirplaneScore 8 0 0 0", // Number of airplanes shot
    END_STATE_DEFINITIONS

    server_lock = new OSMutex();
    server = NULL;
    state_lock = new OSMutex();
    
    // Give the static C-functions a reference to this program
    currentTask = this;
    
    // Start the server on a separate thread
    mg_start_thread(MongooseServerThread, NULL);
}

void
MongooseTask::OnPreflight(const SignalProperties& Input) const {
    if (string(Parameter("FileDirectory")).empty()) {
        bcierr << "Mongoose FileDirectory must be specified" << endl;
    }
    if (string(Parameter("ListeningPort")).empty()) {
        bcierr << "Mongoose server ListeningPort must be specified" << endl;
    }
}

void
DynamicFeedbackTask::OnInitialize(const SignalProperties& Input) {
    
    // Initialize the server
    server_lock->Acquire();
    if (server != NULL) {
        mg_destroy_server(&server);
    }
    server = mg_create_server(NULL);

    // Determine and set where the files to be served are
    char fileDirectory[MAX_PATH];
    fileDirectory = realpath(string(Parameter("FileDirectory")).c_str(), fileDirectory);
    mg_set_option(server, "document_root", fileDirectory);
    
    // Set the request handler
    mg_set_request_handler(server, MongooseServerHandler);
    
    // Determine and set the listening port
    string listeningPort = string(Parameter("ListeningPort"));
    mg_set_option(server, "listening_port", listeningPort.c_str());
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
    // Reset various counters
    ++mRunCount;
    mTrialCount = 0;
    mTrialStatistics.Reset();
    mScore = 0;
    State("GameScore") = mScore;
    
    // Reset state of the score of the game
    server_lock->Acquire();
    countdownMissileScore = 0;
    countdownAirplaneScore = 0;
    isRunning = true;

    // Determine the trial types
    if (nextTrialType != NULL) {
        delete nextTrialType;
    }
    nextTrialType = new queue<TrialType>();
    // Note: This parameter is defined in FeedbackTask.cpp
    if (!string(Parameter("NumberOfTrials")).empty()) {
        // We know the number of trials,
        // so we can enforce an equal number of each trial type
        int numTrials = Parameter("NumberOfTrials");
        int typesPerTrial = numTrials / LAST;
        vector<TrialType> trialVector(numTrials, AIRPLANE);
        for (int i = AIRPLANE + 1; i < LAST; i++) {
            for (int j = 0; j < typesPerTrial; j++) {
                trialVector[i * typesPerTrial + j] = static_cast<TrialType>(i);
            }
        }

        // Shuffle and store the ordering
        random_shuffle(trialVector.begin(), trialVector.end());
        for (unsigned int i = 0; i < trialVector.size(); i++) {
            nextTrialType->push(trialVector[i]);
        }
    }
    lastClientPost = CONTINUE;
    targetHit = false;
    runEnded = false;
    server_lock->Release();

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
        
        // Update the trial type state
        State("TrialType") = (int) currentTrialType;
    }
    server_lock->Release();
}

void
DynamicFeedbackTask::OnTrialBegin() {
    ++mTrialCount;
    AppLog.Screen << "Trial #" << mTrialCount
                  << ", target: " << State("TargetCode")
                  << endl << ", TrialType: " << (int) State("TrialType") << ", " << currentTrialType << endl;

    if (mVisualFeedback == true) {
        mIsVisualCatchTrial = false;
        for (size_t i = 0; i < mVisualCatchTrials.size(); i++) {
            mIsVisualCatchTrial = (mVisualCatchTrials.at(i) == mTrialCount);
        }

        if (mIsVisualCatchTrial == true) {
            AppLog.Screen << "<- visual catch trial" << endl;
        }
    }

    DisplayMessage("");
    RGBColor targetColor = RGBColor(Parameter("TargetColor"));
    for (int i = 0; i < mpFeedbackScene->NumTargets(); ++i) {
        mpFeedbackScene->SetTargetColor(targetColor, i);
        mpFeedbackScene->SetTargetVisible(State("TargetCode") == (i + 1), i);
    }

    // Reset some states to defaults
    server_lock->Acquire();
    targetHit = false;
    countdownSpacebarPressed = false;
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

	State("CountdownMissileScore") = State("CountdownMissileScore") + 1;

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
    if (lastClientPost == STOP_TRIAL) {
        doProgress = true;
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

    mpFeedbackScene->SetCursorVisible(false);

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
    doProgress = false;
    
    // Wait for the stop signal
    // This includes a status update from the game
    server_lock->Acquire();
    if (lastClientPost == STOP_TRIAL) {
        lastClientPost = CONTINUE;

        // Now store the status update into B2B's state
        State("CountdownHitReported") = countdownSpacebarPressed;
        State("CountdownMissileScore") = countdownMissileScore;
        State("CountdownAirplaneScore") = countdownAirplaneScore;
    }

    // Wait for the start signal
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

    server_lock->Acquire();
    runEnded = true;
    isRunning = false;
    server_lock->Release();

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
static void *MongooseServerThread(void *arg) {
    // Keep polling the server
    // Note: the server can be swapped out at any time (hence the locking)
    while (true) {
        if (currentTask->server == NULL) {
            continue;
        }
        
        currentTask->server_lock->Acquire();
        mg_poll_server(currentTask->server, 10);
        currentTask->server_lock->Release();
    }

    return NULL;
}

/*
 * Splits a string by a delimiter
 */
static vector<string> split(const string input, char delimiter) {
    stringstream splitter(input);
    string item;
    vector<string> items;
    while (getline(splitter, item, delimiter)) {
        if (!item.empty()) {
            items.push_back(item);
        }
    }
    return items;
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
            // Check to see if the application is ready
            // Otherwise, reject the REST call
            if (!currentTask->isRunning) {
                mg_send_status(conn, 418);
                mg_send_data(conn, "", 0);
                return MG_REQUEST_PROCESSED;
            }
            
            // Fetch the next trial type
            if (currentTask->nextTrialType->empty()) {
                currentTask->currentTrialType = static_cast<DynamicFeedbackTask::TrialType>(rand() % DynamicFeedbackTask::LAST);
            } else {
                currentTask->currentTrialType = currentTask->nextTrialType->front();
                currentTask->nextTrialType->pop();
            }
            
            mg_send_status(conn, 200);
            mg_send_header(conn, "Content-Type", "text/plain");
            mg_printf_data(conn, "%d", currentTask->currentTrialType);
            currentTask->lastClientPost = DynamicFeedbackTask::START_TRIAL;

            return MG_REQUEST_PROCESSED;

        } else if (uri.compare("/trial/stop") == 0) {
            mg_send_status(conn, 204);
            mg_send_data(conn, "", 0);
            currentTask->lastClientPost = DynamicFeedbackTask::STOP_TRIAL;
            
            // Process the query string
            vector<string> queries = split(string(conn->query_string), '&');
            for (unsigned int i = 0; i < queries.size(); i++) {
                vector<string> parts = split(queries[i], '=');
                if (parts.size() != 2) {
                    continue;
                }
                
                // We expect integers
                if (parts[0].compare("spacebar") == 0) {
                    currentTask->countdownSpacebarPressed = stoi(parts[1]);
                } else if (parts[0].compare("missile") == 0) {
                    currentTask->countdownMissileScore = stoi(parts[1]);
                } else if (parts[0].compare("airplane") == 0) {
                    currentTask->countdownAirplaneScore = stoi(parts[1]);
                }
            }

            return MG_REQUEST_PROCESSED;
        }

    } else if (method.compare("GET") == 0) {
        // This allows the Countdown game to poll for a status
        //   HIT means a TMS pulse should be triggerd
        //   REFRESH means the run has ended and the game should refresh
        if (uri.compare("/trial/status") == 0) {
            if (currentTask->targetHit) {
                mg_send_status(conn, 200);
                mg_send_header(conn, "Content-Type", "text/plain");
                mg_printf_data(conn, "HIT");
                currentTask->targetHit = false;
            } else if (currentTask->runEnded) {
                mg_send_status(conn, 200);
                mg_send_header(conn, "Content-Type", "text/plain");
                mg_printf_data(conn, "REFRESH");
                currentTask->runEnded = false;
            } else {
                mg_send_status(conn, 204);
                mg_send_data(conn, "", 0);
            }

            return MG_REQUEST_PROCESSED;
        }
    }

    return MG_REQUEST_NOT_PROCESSED;
}
