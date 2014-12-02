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
      timeCount(0),
      trialDelayActive(false), 
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
    "Application:Sequencing float QuestionPreviewTime= 2.0s 2.0s 0 % "
        " // Time that a trial's question is displayed before the trial begins",
    "Application:Sequencing float TrialStartDelay= 1.0s 1.0s 0 % "
        " // Time after displaying a trial's question but before the start of the trial",
    "Application:UI int CursorVisible= 1 1 0 1"
        " // Do you want to see the vertical cursor? 0: No; 1: Yes",
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
    Parameter("CursorVisible");

    int feedbackDuration = static_cast<int>(Parameter("FeedbackDuration").InSampleBlocks());
    int dwellTime = static_cast<int>(Parameter("DwellTime").InSampleBlocks());
    if (dwellTime > feedbackDuration / 2) {
        bcierr << "Dwell time must be less than half of the feedback duration" << std::endl;
    }

    int questionPreviewTime = static_cast<int>(Parameter("QuestionPreviewTime").InSampleBlocks());
    if (questionPreviewTime > feedbackDuration / 2) {
        bcierr << "QuestionPreviewTime must be less than half of the feedback duration" << std::endl;
    }

    int trialStartDelay = static_cast<int>(Parameter("TrialStartDelay").InSampleBlocks());
    if (trialStartDelay > questionPreviewTime / 2) {
        bcierr << "TrialStartDelay must be less than half of QuestionPreviewTime" << std::endl;
    }

    CheckServerParameters(Input);
}

void Brain2Brain::OnInitialize(const SignalProperties& Input) {
    InitializeServer(Input, 2);

    // Reset the GUI
    delete B2BGUI;
    B2BGUI = new Brain2BrainUI(window);
    B2BGUI->Initialize();
    
    // These parameters are used in the asynchronous web handlers, so they need to be copied
    questionPreviewTimeMs = static_cast<int>(Parameter("QuestionPreviewTime").InMilliseconds());
    trialStartDelayMs = static_cast<int>(Parameter("TrialStartDelay").InMilliseconds());
}

void Brain2Brain::OnStartRun() {
    MongooseOnStartRun();

    // Reset or increment some counters
    runCount++;
    trialCount = 0;
    timeCount = 0;
    trialDelayActive = false;

    AppLog << "Run #" << runCount << " started" << std::endl;
    B2BGUI->OnStartRun();
}

void Brain2Brain::DoPreRun(const GenericSignal&, bool& doProgress) {
    // Wait for the start signal
    doProgress = false;
    
    if (trialDelayActive) {
        int questionPreviewTime = static_cast<int>(Parameter("QuestionPreviewTime").InSampleBlocks());
        int trialStartDelay = static_cast<int>(Parameter("TrialStartDelay").InSampleBlocks());
        
        // This is between the question preview and the trial start delay
        if (timeCount == questionPreviewTime) {
            B2BGUI->HideQuestion();
        }
        
        if (timeCount >= questionPreviewTime + trialStartDelay) {
            doProgress = true;
            trialDelayActive = false;
        }
        
        timeCount++;
    } else {
        state_lock->Acquire();

        // Acknowledge the start signal and start the delay phase (above)
        if (lastClientPost == START_TRIAL) {
            lastClientPost = CONTINUE;
            timeCount = 0;
            trialDelayActive = true;
            B2BGUI->ShowQuestion();
        }
        
        state_lock->Release();
    }
}

void Brain2Brain::OnTrialBegin() {
    // Reset trial-specific state
    state_lock->Acquire();
    targetHit = false;
    targetCollisionDetected = false;
    state_lock->Release();
    State("TargetHitCode") = 0;
    
    // Increment the trial count
    trialCount++;

    AppLog << "Trial #" << trialCount << std::endl;
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
        targetCollisionDetected = true;
        targetHitType = hitType;
        state_lock->Release();
    }

    // Either the YES or NO target was hit
    doProgress = true;
}

void Brain2Brain::OnFeedbackEnd() {
    if (!targetCollisionDetected) {
        // We always want to trigger the TMS
        // In this case, the target wasn't hit before the end of the trial
        //   so we artificially "hit" the closer target
        state_lock->Acquire();
        Brain2BrainUI::TargetHitType hitType = B2BGUI->GetClosestTarget();
        AppLog << "Chose closest target" << std::endl;
        State("TargetHitCode") = static_cast<long>(targetHitType);
        targetHit = true;
        targetHitType = hitType;
        state_lock->Release();
    }
    
    B2BGUI->OnFeedbackEnd();

    // Clear the question box between trials
    B2BGUI->HideQuestion();
}

void Brain2Brain::DoITI(const GenericSignal& signal, bool& doProgress) {
    // There's no distinction between the ITI and the PreRun
    // Both phases are considered to be "Pre-trial" setup
    DoPreRun(signal, doProgress);
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

void Brain2Brain::HandleTrialStartRequest(std::string data, int &trialDuration) {
    B2BGUI->SetQuestion(data);
    
    // To the default trial duration, add the pre-trial delay period
    trialDuration += questionPreviewTimeMs + trialStartDelayMs;
}

void Brain2Brain::HandleAnswerUpdate(std::string data) {
    B2BGUI->SetAnswer(data);
}
