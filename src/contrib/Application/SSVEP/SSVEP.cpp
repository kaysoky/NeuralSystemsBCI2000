#include "PCHIncludes.h"
#pragma hdrstop

#include "SSVEP.h"

#include <stdio.h>
#include <math.h>

RegisterFilter( SSVEPFeedbackTask, 3 );

SSVEPFeedbackTask::SSVEPFeedbackTask() {
      
    // Note: See MongooseTask.cpp for more parameters and states
    
    BEGIN_PARAMETER_DEFINITIONS
    "Application:Targets matrix Targets= "
        " 2 " // rows
        " [pos%20x pos%20y] " // columns
        " 10 50 "
        " 90 50 "
        " // Number of targets and their position in percentage coordinates",
    "Application:Targets int TargetColor= 0x0000FF % % % " // Blue
        " // target color (color)",
    END_PARAMETER_DEFINITIONS

    BEGIN_STATE_DEFINITIONS
        "GameScore 16 0 0 0",
    END_STATE_DEFINITIONS
               
    state_lock = new OSMutex();
}

SSVEPFeedbackTask::~SSVEPFeedbackTask() {}

void
SSVEPFeedbackTask::OnPreflight(const SignalProperties& Input) const {
    if (Parameter("Targets")->NumValues() <= 0) {
        bcierr << "At least one target must be specified" << std::endl;
    }
    
    const char* colorParams[] = {
        "TargetColor"
    };
    for (size_t i = 0; i < sizeof(colorParams) / sizeof(*colorParams); ++i) {
        if (RGBColor(Parameter(colorParams[i])) == RGBColor(RGBColor::NullColor)) {
            bcierr << "Invalid RGB value in " << colorParams[ i ] << std::endl;
        }
    }
    
    CheckServerParameters(Input);
}

void
SSVEPFeedbackTask::OnInitialize(const SignalProperties& Input) {
    InitializeServer(Input);
}

void
SSVEPFeedbackTask::OnStartRun() {
    // Reset state of the game
    state_lock->Acquire();
    state_lock->Release();
    
    // Reset various counters
}

void
SSVEPFeedbackTask::DoPreRun(const GenericSignal&, bool& doProgress) {
    // Wait for the start signal
    doProgress = false;
}

void
SSVEPFeedbackTask::OnTrialBegin() {
    // Reset trial-specific Countdown state
    state_lock->Acquire();
    state_lock->Release();
}

void
SSVEPFeedbackTask::OnFeedbackBegin() {
}

void
SSVEPFeedbackTask::DoFeedback(const GenericSignal& ControlSignal, bool& doProgress) {
    doProgress = false;
    
    // Save whatever state is available to BCI2000
    state_lock->Acquire();

    // Check for the stop signal
    
    state_lock->Release();
}

void
SSVEPFeedbackTask::OnFeedbackEnd() {}

void
SSVEPFeedbackTask::OnTrialEnd(void) { };

void
SSVEPFeedbackTask::DoITI(const GenericSignal&, bool& doProgress) {
    // Wait for the start signal
    doProgress = false;

}

void
SSVEPFeedbackTask::OnStopRun() {
    // Indicate that the run has ended
    state_lock->Acquire();
    state_lock->Release();
}

int SSVEPFeedbackTask::HandleMongooseRequest(struct mg_connection *conn) {
    std::string method(conn->request_method);
    std::string uri(conn->uri);

    state_lock->Acquire();
    if (method.compare("POST") == 0) {
        if (uri.compare("") == 0) {
            //TODO
            
            state_lock->Release();
            return MG_REQUEST_PROCESSED;

        }

    } else if (method.compare("GET") == 0) {
    }
    
    state_lock->Release();
    return MG_REQUEST_NOT_PROCESSED;
}

