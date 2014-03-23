#ifndef SSVEP_H
#define SSVEP_H

#include "MongooseFeedbackTask.h"
#include "ApplicationWindow.h"
#include "OSMutex.h"
#include "mongoose.h"
#include "SSVEPUI.h"

class SSVEPFeedbackTask : public MongooseFeedbackTask {
public:
    SSVEPFeedbackTask();
    virtual ~SSVEPFeedbackTask();
    
    virtual int HandleMongooseRequest(struct mg_connection *conn);
    
private:
    // Startup events
    virtual void OnPreflight(const SignalProperties& Input) const;
    virtual void OnInitialize(const SignalProperties& Input);
    virtual void OnStartRun();
    virtual void DoPreRun(const GenericSignal&, bool& doProgress);

    // Trial Loop
    virtual void OnTrialBegin();
    virtual void DoPreFeedback(const GenericSignal&, bool& doProgress) { doProgress = true; };
    virtual void OnFeedbackBegin();
    virtual void DoFeedback(const GenericSignal&, bool& doProgress);
    virtual void OnFeedbackEnd();
    virtual void DoPostFeedback(const GenericSignal&, bool& doProgress) { doProgress = true; };
    virtual void OnTrialEnd() {};
    virtual void DoITI(const GenericSignal&, bool& doProgress);

    // Cleanup events
    virtual void OnStopRun();
    virtual void OnHalt() {};

    /////////////////////////
    // Brain2Brain Objects //
    /////////////////////////

    // Graphics objects
    ApplicationWindow &window;
    SSVEPUI *SSVEPGUI;

    int runCount,
        trialCount;
    
    ///////////////////////////////
    // 20 Questions game objects //
    ///////////////////////////////
    
    
};

#endif // SSVEP_H

