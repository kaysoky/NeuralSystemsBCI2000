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
        " // Frequency of input expected for each target and their position in percentage coordinates", 
    "Application:SSVEP float ArrowLength= 10 % 0 100 " 
        " // Length of an arrow in percent of screen dimensions"
    END_PARAMETER_DEFINITIONS
}

SSVEPFeedbackTask::~SSVEPFeedbackTask() {
	delete SSVEPGUI;
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
    
    Parameter("ArrowLength");

    CheckServerParameters(Input);
}

void
SSVEPFeedbackTask::OnInitialize(const SignalProperties& Input) {
    InitializeServer(Input, Parameter("Arrows")->NumRows());

    // Reset the GUI
    delete SSVEPGUI;
    SSVEPGUI = new SSVEPUI(window);
    SSVEPGUI->Initialize();
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

    AppLog.Screen << "Trial #" << trialCount << " => " << Parameter("Arrows")(currentTrialType, 0) << " Hz" << std::endl;
    SSVEPGUI->OnTrialBegin();
    SSVEPGUI->ShowArrow(currentTrialType);
}

void
SSVEPFeedbackTask::DoFeedback(const GenericSignal& ControlSignal, bool& doProgress) {
    doProgress = false;

    //TODO: Train or Classify
    
    // Check for the stop signal
    state_lock->Acquire();
    if (lastClientPost == STOP_TRIAL) {
        doProgress = true;
        lastClientPost = CONTINUE;
    }
    state_lock->Release();
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
