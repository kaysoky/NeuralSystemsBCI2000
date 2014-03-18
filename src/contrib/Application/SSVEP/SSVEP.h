#ifndef SSVEP_H
#define SSVEP_H

#include "MongooseFeedbackTask.h"
#include "ApplicationWindow.h"
#include "OSMutex.h"

class SSVEPFeedbackTask : public MongooseFeedbackTask {
public:
    SSVEPFeedbackTask();
    virtual ~SSVEPFeedbackTask();
    
    virtual int HandleMongooseRequest(struct mg_connection *conn);
    
private:
    ///////////////////////
    // FeedbackTask Loop //
    ///////////////////////
    // See: http://www.bci2000.org/wiki/index.php/Programming_Reference:FeedbackTask_Class#Events_Summary
    // Note: any method with a 'doProgress' bool can loop when set to false

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
    virtual void OnTrialEnd();
    virtual void DoITI(const GenericSignal&, bool& doProgress);

    // Cleanup events
    virtual void OnStopRun();
    virtual void OnHalt() {};

    /////////////////////////
    // Brain2Brain Objects //
    /////////////////////////
    
    //TODO
    
    ///////////////////////////////
    // 20 Questions game objects //
    ///////////////////////////////
    
    /*
     * Provides synchronization for the 20 Questions game state
     * Any function that touches private variables of this class must first acquire this lock
     * Note: Reading does not (in most cases) require locking
     */
    OSMutex *state_lock;
    
    //TODO
};

#endif // SSVEP_H

