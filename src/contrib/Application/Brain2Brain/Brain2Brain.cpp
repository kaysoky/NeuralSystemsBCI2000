#include "PCHIncludes.h"
#pragma hdrstop

#include "Brain2Brain.h"

#include "buffers.h"
#include <stdio.h>
#include <algorithm>
#include <cstdlib>

RegisterFilter( Brain2Brain, 3 );

Brain2Brain::Brain2Brain()
    : B2BGUI(NULL),
      window(Window()),
      runCount(0),
      nextTrialType(NULL) {

    // Note: See MongooseTask.cpp for more parameters and states

    BEGIN_PARAMETER_DEFINITIONS
    "Application:UI float CursorWidth= 10 % 0 100 "
        " // Diameter of the feedback cursor width as a percent of screen width",
    "Application:UI float TargetHeight= 10 % 0 100 "
        " // Height of each of the targets as a percent of screen height",
    END_PARAMETER_DEFINITIONS

    state_lock = new OSMutex();
}

Brain2Brain::~Brain2Brain() {
	// TODO: Cleanup is not thread-safe yet
    // delete state_lock;
    delete B2BGUI;
    delete nextTrialType;
}

void Brain2Brain::OnPreflight(const SignalProperties& Input) const {
    // The values of these parameters are bounded by definition
    Parameter("CursorWidth");
    Parameter("TargetHeight");

    CheckServerParameters(Input);
}

void Brain2Brain::OnInitialize(const SignalProperties& Input) {
    InitializeServer(Input);

    // Reset the GUI
    delete B2BGUI;
    B2BGUI = new Brain2BrainUI(window);
    B2BGUI->Initialize();
}

void Brain2Brain::OnStartRun() {
    // Reset state of the game
    state_lock->Acquire();
    lastClientPost = CONTINUE;
    isRunning = true;
    runEnded = false;

    // Determine the trial types
    if (nextTrialType != NULL) {
        delete nextTrialType;
    }
    nextTrialType = new std::queue<TrialType>();
    // Note: This parameter is defined in FeedbackTask.cpp
    if (!std::string(Parameter("NumberOfTrials")).empty()) {
        // We know the number of trials,
        // so we can enforce an equal number of each trial type
        int numTrials = Parameter("NumberOfTrials");
        int typesPerTrial = numTrials / LAST;
        std::vector<TrialType> trialVector(numTrials, AIRPLANE);
        for (int i = AIRPLANE + 1; i < LAST; i++) {
            for (int j = 0; j < typesPerTrial; j++) {
                trialVector[i * typesPerTrial + j] = static_cast<TrialType>(i);
            }
        }

        // Shuffle and store the ordering
        std::random_shuffle(trialVector.begin(), trialVector.end());
        for (unsigned int i = 0; i < trialVector.size(); i++) {
            nextTrialType->push(trialVector[i]);
        }
    }
    state_lock->Release();

    // Reset or increment some counters
    runCount++;
    trialCount = 0;

    AppLog << "Run #" << runCount << " started" << std::endl;
    B2BGUI->OnStartRun();
}

void Brain2Brain::DoPreRun(const GenericSignal&, bool& doProgress) {
    // Wait for the start signal
    doProgress = false;
	
	state_lock->Acquire();
    if (lastClientPost == START_TRIAL) {
        doProgress = true;
		lastClientPost = CONTINUE;
    }
	state_lock->Release();
}

void Brain2Brain::OnTrialBegin() {
    // Reset trial-specific Countdown state
    state_lock->Acquire();
    targetHit = false;
    state_lock->Release();

    // Increment the trial count
    trialCount++;

    AppLog.Screen << "Trial #" << trialCount << std::endl;
    B2BGUI->OnTrialBegin();
}

void Brain2Brain::OnFeedbackBegin() {
    B2BGUI->OnFeedbackBegin();
}

void Brain2Brain::DoFeedback(const GenericSignal& ControlSignal, bool& doProgress) {
    doProgress = false;

    Brain2BrainUI::TargetHitType targetHitType = B2BGUI->DoFeedback(ControlSignal);
    if (targetHitType == Brain2BrainUI::NOTHING_HIT) {
        // Check for the stop signal
        state_lock->Acquire();
        if (lastClientPost == STOP_TRIAL) {
            doProgress = true;
			lastClientPost == CONTINUE;
        }
        state_lock->Release();
        
        return;
    }
    
    if (targetHitType == Brain2BrainUI::YES_TARGET) {
        // Pass this information onto the Countdown client
        state_lock->Acquire();
        targetHit = true;
        state_lock->Release();
    }
    
    // Either the YES or NO target was hit
    doProgress = true;
}

void
Brain2Brain::OnFeedbackEnd() {
    B2BGUI->OnFeedbackEnd();
}

void
Brain2Brain::OnTrialEnd(void) { };

void
Brain2Brain::DoITI(const GenericSignal&, bool& doProgress) {
    doProgress = false;

    // Wait for the start signal
	state_lock->Acquire();
    if (lastClientPost == START_TRIAL) {
        doProgress = true;
		lastClientPost = CONTINUE;
    }
	state_lock->Release();
}

void
Brain2Brain::OnStopRun() {
    AppLog << "Run " << runCount << " finished: "
           << trialCount << " trial(s)" << std::endl;

    // Indicate that the run has ended
    state_lock->Acquire();
    runEnded = true;
    isRunning = false;
    state_lock->Release();

    B2BGUI->OnStopRun();
}

void Brain2Brain::HandleTrialStartRequest(struct mg_connection *conn) {
    // Fetch the next trial type
    if (nextTrialType->empty()) {
        currentTrialType = static_cast<TrialType>(rand() % LAST);
    } else {
        currentTrialType = nextTrialType->front();
        nextTrialType->pop();
    }

    // Pass along the trial type
    mg_send_status(conn, 200);
    mg_send_header(conn, "Content-Type", "text/plain");
    mg_printf_data(conn, "%d", currentTrialType);
    lastClientPost = START_TRIAL;
}

void Brain2Brain::HandleTrialStopRequest(struct mg_connection *conn) {
    mg_send_status(conn, 204);
    mg_send_data(conn, "", 0);
    lastClientPost = STOP_TRIAL;
}

void Brain2Brain::HandleTrialStatusRequest(struct mg_connection *conn) {
    if (targetHit) {
        mg_send_status(conn, 200);
        mg_send_header(conn, "Content-Type", "text/plain");
        mg_printf_data(conn, "HIT");
        targetHit = false;
    } else if (runEnded) {
        mg_send_status(conn, 200);
        mg_send_header(conn, "Content-Type", "text/plain");
        mg_printf_data(conn, "REFRESH");
        runEnded = false;
    } else {
        mg_send_status(conn, 204);
        mg_send_data(conn, "", 0);
    }
}

int Brain2Brain::HandleMongooseRequest(struct mg_connection *conn) {
    std::string method(conn->request_method);
    std::string uri(conn->uri);

    state_lock->Acquire();
    if (method.compare("POST") == 0) {
        if (uri.compare("/trial/start") == 0) {
            HandleTrialStartRequest(conn);

            state_lock->Release();
            return MG_REQUEST_PROCESSED;

        } else if (uri.compare("/trial/stop") == 0) {
            HandleTrialStopRequest(conn);

            state_lock->Release();
            return MG_REQUEST_PROCESSED;
        }

    } else if (method.compare("GET") == 0) {
        if (uri.compare("/trial/status") == 0) {
            HandleTrialStatusRequest(conn);

            state_lock->Release();
            return MG_REQUEST_PROCESSED;
        }
    }

    state_lock->Release();
    return MG_REQUEST_NOT_PROCESSED;
}

