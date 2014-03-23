#include "PCHIncludes.h"
#pragma hdrstop

#include "Brain2Brain.h"

#include "buffers.h"
#include <stdio.h>

RegisterFilter( Brain2Brain, 3 );

const char* Brain2Brain::TrialTypeText[] = { "AIRPLANE", "MISSILE" };

Brain2Brain::Brain2Brain()
    : B2BGUI(NULL),
      window(Window()),
      runCount(0), 
      targetHit(false) {

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
}

void Brain2Brain::OnPreflight(const SignalProperties& Input) const {
    // The values of these parameters are bounded by definition
    Parameter("CursorWidth");
    Parameter("TargetHeight");

    CheckServerParameters(Input);
}

void Brain2Brain::OnInitialize(const SignalProperties& Input) {
    InitializeServer(Input, static_cast<int>(LAST));

    // Reset the GUI
    delete B2BGUI;
    B2BGUI = new Brain2BrainUI(window);
    B2BGUI->Initialize();
}

void Brain2Brain::OnStartRun() {
    MongooseOnStartRun();

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

    AppLog.Screen << "Trial #" << trialCount << " => " << TrialTypeText[currentTrialType] << std::endl;
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
			lastClientPost = CONTINUE;
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

    MongooseOnStopRun();
    B2BGUI->OnStopRun();
}

bool Brain2Brain::HandleTrialStatusRequest(struct mg_connection *conn) {
    if (targetHit) {
        mg_send_status(conn, 200);
        mg_send_header(conn, "Content-Type", "text/plain");
        mg_printf_data(conn, "HIT");
        targetHit = false;
        return true;
    } 
    
    return false;
}
