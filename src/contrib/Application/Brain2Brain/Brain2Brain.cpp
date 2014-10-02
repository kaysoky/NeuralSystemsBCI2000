#include "PCHIncludes.h"
#pragma hdrstop

#include "Brain2Brain.h"

#include "buffers.h"
#include <stdio.h>

RegisterFilter( Brain2Brain, 3 );

Brain2Brain::Brain2Brain()
    : B2BGUI(NULL),
      window(Window()),
      runCount(0), 
      targetHit(false), 
      targetHitType(Brain2BrainUI::NOTHING_HIT) {

    // Note: See MongooseTask.cpp for more parameters and states

    BEGIN_PARAMETER_DEFINITIONS
    "Application:UI float CursorWidth= 10 % 0 100 "
        " // Diameter of the feedback cursor width as a percent of screen width",
    "Application:UI float TargetHeight= 10 % 0 100 "
        " // Height of each of the targets as a percent of screen height",
    "Application:UI float DwellTime= 0.25s 0.25s 0 % "
        " // Time that the cursor must dwell over a target to be considered a hit", 
    END_PARAMETER_DEFINITIONS

    BEGIN_STATE_DEFINITIONS
    "CursorCenter 32 0 0 0" // A (32-bit) integer
    END_STATE_DEFINITIONS
}

Brain2Brain::~Brain2Brain() {
    delete B2BGUI;
}

void Brain2Brain::OnPreflight(const SignalProperties& Input) const {
    // The values of these parameters are bounded by definition
    Parameter("CursorWidth");
    Parameter("TargetHeight");
    
    int feedbackDuration = static_cast<int>(Parameter( "FeedbackDuration" ).InSampleBlocks());
    int dwellTime = static_cast<int>(Parameter("DwellTime").InSampleBlocks());
    if (dwellTime > feedbackDuration / 2) {
        bcierr << "Dwell time must be less than half of the feedback duration" << std::endl;
    }

    CheckServerParameters(Input);
}

void Brain2Brain::OnInitialize(const SignalProperties& Input) {
    InitializeServer(Input, 2);

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
    // Reset trial-specific state
    state_lock->Acquire();
    targetHit = false;
    state_lock->Release();
    State("TargetHitCode") = 0;

    // Increment the trial count
    trialCount++;

    AppLog << "Trial #" << trialCount << std::endl;
    B2BGUI->OnTrialBegin();
}

void Brain2Brain::OnFeedbackBegin() {
    B2BGUI->OnFeedbackBegin();
}

void Brain2Brain::DoFeedback(const GenericSignal& ControlSignal, bool& doProgress) {
    doProgress = false;

    Brain2BrainUI::TargetHitType hitType = B2BGUI->DoFeedback(ControlSignal);
    State("TargetHitCode") = static_cast<long>(targetHitType);

    if (hitType == Brain2BrainUI::NOTHING_HIT) {
        // Check for the stop signal
        state_lock->Acquire();
        if (lastClientPost == STOP_TRIAL) {
            doProgress = true;
            lastClientPost = CONTINUE;
        }
        state_lock->Release();
        
        return;
    } else {
        // Pass this information onto the Countdown client
        state_lock->Acquire();
        targetHit = true;
        targetHitType = hitType;
        state_lock->Release();
    }
    
    // Either the YES or NO target was hit
    doProgress = true;
}

void Brain2Brain::OnFeedbackEnd() {
    B2BGUI->OnFeedbackEnd();

    // Clear the question box between trials
    B2BGUI->SetQuestion("");
}

void Brain2Brain::DoITI(const GenericSignal&, bool& doProgress) {
    doProgress = false;

    // Wait for the start signal
    state_lock->Acquire();
    if (lastClientPost == START_TRIAL) {
        doProgress = true;
        lastClientPost = CONTINUE;
    }
    state_lock->Release();
}

void Brain2Brain::OnStopRun() {
    AppLog << "Run " << runCount << " finished: "
           << trialCount << " trial(s)" << std::endl;

    MongooseOnStopRun();
    B2BGUI->OnStopRun();
}

bool Brain2Brain::HandleTrialStatusRequest(struct mg_connection *conn) {
    if (targetHit) {
        mg_send_status(conn, 200);
        mg_send_header(conn, "Content-Type", "text/plain");
        switch (targetHitType) {
        case Brain2BrainUI::YES_TARGET:
            mg_printf_data(conn, "YES");
            break;
        case Brain2BrainUI::NO_TARGET:
            mg_printf_data(conn, "NO");
            break;
        default:
            AppLog << "Fatal logic error!  Target hit is not a yes or no.";
            break;
        }
        targetHit = false;
        return true;
    } 
    
    return false;
}

void Brain2Brain::HandleTrialStartRequest(std::string data) {
    B2BGUI->SetQuestion(data);
}

void Brain2Brain::HandleAnswerUpdate(std::string data) {
    B2BGUI->SetAnswer(data);
}
