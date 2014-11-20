#ifndef BRAIN2BRAIN_H
#define BRAIN2BRAIN_H

#include "MongooseFeedbackTask.h"
#include "ApplicationWindow.h"
#include "mongoose.h"
#include "Brain2BrainUI.h"

class Brain2Brain : public MongooseFeedbackTask {
public:
    Brain2Brain();
    virtual ~Brain2Brain();
    
private:
    // Startup events
    virtual void OnPreflight(const SignalProperties&) const;
    virtual void OnInitialize(const SignalProperties&);
    virtual void OnStartRun();
    virtual void DoPreRun(const GenericSignal&, bool&);

    // Trial Loop
    virtual void OnTrialBegin();
    virtual void DoPreFeedback(const GenericSignal&, bool& doProgress) { doProgress = true; };
    virtual void OnFeedbackBegin();
    virtual void DoFeedback(const GenericSignal&, bool&);
    virtual void OnFeedbackEnd();
    virtual void DoPostFeedback(const GenericSignal&, bool& doProgress) { doProgress = true; };
    virtual void OnTrialEnd() {};
    virtual void DoITI(const GenericSignal&, bool&);

    // Cleanup events
    virtual void OnStopRun();
    virtual void OnHalt() {};

    /////////////////////////
    // Brain2Brain Objects //
    /////////////////////////

    // Graphics objects
    ApplicationWindow &window;
    Brain2BrainUI *B2BGUI;

    int runCount,
        timeCount,
        trialCount;
    
    virtual void HandleTrialStartRequest(std::string);
    virtual bool HandleTrialStatusRequest(struct mg_connection *);
    virtual void HandleAnswerUpdate(std::string);

    /*
     * Holds whether the YES target has been hit
     * The game is expected to poll for this value regularly
     * When it is sent to the game, the value is reset to false.
     */
    bool targetHit;
    Brain2BrainUI::TargetHitType targetHitType;
};

#endif // BRAIN2BRAIN_H

