#include "PCHIncludes.h"
#pragma hdrstop

#include "SSVEP.h"

#include <stdio.h>
#include <math.h>

RegisterFilter( SSVEPFeedbackTask, 3 );

SSVEPFeedbackTask::SSVEPFeedbackTask() 
    : SSVEPGUI(NULL),
      window(Window()),
      runCount(0) {

    // Note: See MongooseTask.cpp for more parameters and states

    BEGIN_PARAMETER_DEFINITIONS
    "Application:SSVEP matrix Arrows= "
        " 2 " // rows
        " [Frequency X Y] " // columns
        " 12 10 50 "
        " 17 90 50 "
        " // Frequency of input expected for each target and their position in percentage coordinates"
    END_PARAMETER_DEFINITIONS

    state_lock = new OSMutex();
}

SSVEPFeedbackTask::~SSVEPFeedbackTask() {
	// TODO: Cleanup is not thread-safe yet
    // delete state_lock;
}

void
SSVEPFeedbackTask::OnPreflight(const SignalProperties& Input) const {
    if (Parameter("Arrows")->NumRows() <= 1) {
        bcierr << "At least two target frequencies must be specified" << std::endl;
    }
    if (Parameter("Arrows")->NumColumns() != 3) {
        bcierr << "Target matrix must have 3 columns "
               << "corresponding to the target frequency "
               << "and (X, Y) positions" << std::endl;
    }

    CheckServerParameters(Input);
}

void
SSVEPFeedbackTask::OnInitialize(const SignalProperties& Input) {
    InitializeServer(Input, Parameter("Arrows")->NumRows());

    // Reset the GUI
    delete SSVEPGUI;
    SSVEPGUI = new SSVEPUI(window);
    SSVEPGUI->Initialize();
    
    // ParamRef Targets = Parameter("Targets");
    // for (int i = 0; i < Parameter("Targets")->NumRows(); ++i) {
    //     // Determine the target's dimensions
    //     EllipticShape* pTarget = new GradientEllipticShape(mDisplay);
    //     GUI::Point targetDiag = {Targets(i, dx), Targets(i, dy)};
    //     SceneToObjectCoords(targetDiag, vector);
    //     
    //     // Set the target's size
    //     GUI::Rect targetRect = {0, 0, fabs(targetDiag.x), fabs(targetDiag.y)};
    //     pTarget->SetObjectRect(targetRect);
    //     
    //     // Set the target's origin
    //     GUI::Point targetCenter = {Targets(i, x), Targets(i, y)};
    //     SceneToObjectCoords(targetCenter, point);
    //     pTarget->SetCenter(targetCenter);
    //     
    //     // Hide and save the target
    //     pTarget->Hide();
    //     mTargets.push_back(pTarget);
    // }
}

void
SSVEPFeedbackTask::OnStartRun() {
    // Reset state of the game
    MongooseOnStartRun();

    // Reset or increment some counters
    runCount++;
    trialCount = 0;

    AppLog << "Run #" << runCount << " started" << std::endl;
    SSVEPGUI->OnStartRun();
}

void
SSVEPFeedbackTask::DoPreRun(const GenericSignal&, bool& doProgress) {
    // Wait for the start signal
    doProgress = false;
	
	state_lock->Acquire();
    if (lastClientPost == START_TRIAL) {
        doProgress = true;
		lastClientPost = CONTINUE;
    }
	state_lock->Release();
}

void
SSVEPFeedbackTask::OnTrialBegin() {
    // Reset trial-specific 20 questions state
    state_lock->Acquire();
    state_lock->Release();

    // Increment the trial count
    trialCount++;

    AppLog.Screen << "Trial #" << trialCount << " => ???" << std::endl;
    SSVEPGUI->OnTrialBegin();
}

void
SSVEPFeedbackTask::OnFeedbackBegin() {
    SSVEPGUI->OnFeedbackBegin();
}

void
SSVEPFeedbackTask::DoFeedback(const GenericSignal& ControlSignal, bool& doProgress) {
    doProgress = false;

    //TODO: Train or Classify
    
    // Check for the stop signal
    state_lock->Acquire();
    if (lastClientPost == STOP_TRIAL) {
        doProgress = true;
        lastClientPost == CONTINUE;
    }
    state_lock->Release();
}

void
SSVEPFeedbackTask::OnFeedbackEnd() {
    SSVEPGUI->OnFeedbackEnd();
}

void
SSVEPFeedbackTask::DoITI(const GenericSignal&, bool& doProgress) {
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
SSVEPFeedbackTask::OnStopRun() {
    AppLog << "Run " << runCount << " finished: "
           << trialCount << " trial(s)" << std::endl;

    MongooseOnStopRun();
    SSVEPGUI->OnStopRun();
}
