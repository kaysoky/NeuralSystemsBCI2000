#ifndef BRAIN2BRAIN_H
#define BRAIN2BRAIN_H

#include <queue>
#include <vector>

#include "MongooseFeedbackTask.h"
#include "ApplicationWindow.h"
#include "OSMutex.h"
#include "mongoose.h"
#include "Brain2BrainUI.h"

class Brain2Brain : public MongooseFeedbackTask {
public:
    Brain2Brain();
    virtual ~Brain2Brain();
    
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

    // Graphics objects
    ApplicationWindow &window;
    Brain2BrainUI *B2BGUI;

    int runCount,
        trialCount;
    
    ////////////////////////////
    // Countdown game objects //
    ////////////////////////////
    
    /*
     * Tells the application to start a trial
     * Returns the trial type as an integer
     */
    void HandleTrialStartRequest(struct mg_connection *conn);
    
    /*
     * Tells the application to stop a trial
     * Should also contain a query string with a score update
     * i.e. POST /trial/stop?spacebar=1&missile=0&airplane=1
     */
    void HandleTrialStopRequest(struct mg_connection *conn);
    
    /* This allows the Countdown game to poll for a status
     *   HIT means a TMS pulse should be triggered
     *   REFRESH means the run has ended and the game should refresh
     */   
    void HandleTrialStatusRequest(struct mg_connection *conn);
    
    /*
     * Provides synchronization for the Countdown game state
     * Any function that touches private variables of this class must first acquire this lock
     * Note: Reading does not (in most cases) require locking
     */
    OSMutex *state_lock;
    
    /*
     * Which projectile should be flying across the Countdown game screen on this trial?
     */
    enum TrialType {
        AIRPLANE,
        MISSILE,
        LAST
    };

    /*
     * Should a trial start, stop, or continue?
     */
    enum TrialState {
        START_TRIAL,
        STOP_TRIAL,
        CONTINUE
    };
    
    /*
     * Holds a random list of trials to pass onto the Countdown game
     * If empty, then a random trial type will be chosen instead
     *
     * This should be initialized when a run starts.
     *
     * When the Countdown game starts a trial, it should fetch the trial type from here.
     */
    std::queue<TrialType> *nextTrialType;

    /*
     * Holds the most recent trial-state command issued by the Countdown game
     * A state of CONTINUE means that this value has been processed
     *   and is awaiting a new command from the game.
     */
    TrialState lastClientPost;

    /*
     * This value is updated when the Countdown game calls POST /trial/start
     * This value is also resident in BCI2000's state ("TrialType")
     */
    TrialType currentTrialType;

    /*
     * Holds whether the YES target has been hit
     * The Countdown game is expected to poll for this value regularly
     * When it is sent to the game, the value is reset to false.
     */
    bool targetHit;

    /*
     * Holds whether a set of trials has ended
     * The Countdown game is expected to poll for this value regularly
     * When it is sent to the game, the value is reset to false.
     */
    bool runEnded;
};

#endif // BRAIN2BRAIN_H

