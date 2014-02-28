#ifndef MONGOOSETASK_H
#define MONGOOSETASK_H

#include <queue>
#include <vector>

#include "FeedbackTask.h"

// For interfacing with the Countdown Game
extern "C" {
  #include "mongoose.h"
}
#include "OSMutex.h"

class MongooseTask : public FeedbackTask {
public:
    MongooseTask();
    virtual ~MongooseTask() {};
    
    friend int MongooseServerHandler(struct mg_connection *conn);
	
private:
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
    
    virtual void OnPreflight(const SignalProperties& Input) const;
    virtual void OnInitialize(const SignalProperties& Input);
    
    /* 
     * Provides synchronization for the Mongoose server
     */
    OSMutex *server_lock;
    
    /*
     * Mongoose server
     * Serves up the files within the directory it is given
     *   and handles the various communication REST functions
     * A new server instance is created every time the configuration is set
     */
	struct mg_server *server;
    
    /*
     * Provides synchronization for this class's state
     * Any function that touches private variables of this class must first acquire this lock
     * Note: Reading does not require locking
     */
    OSMutex *state_lock;
    
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
     * Note: This is necessary since the C code should not set the BCI2000 states directly
     * 
     * This value is updated when the Countdown game calls POST /trial/start
     * After lastClientPost is set to START_TRIAL, 
     *   this value should be copied into BCI2000's state ("TrialType")
     */
    TrialType currentTrialType;
    
    /*
     * State from the Countdown game reported when issuing the stop command
     * The two scores should be derivable via the spacebar boolean
     *   but are included for ease of analysis.
     * 
     * Note: Treat these values as semaphores, 
     *         locked by lastClientPost == STOP_TRIAL
     */
    bool countdownSpacebarPressed;
    int countdownMissileScore, countdownAirplaneScore;
    
    /*
     * Holds whether the YES target has been hit
     * The Countdown game is expected to poll for this value regularly
     * When it is sent to the game, the value is reset to false.  
     * 
     * Note: This is different from checking:
     *   State("ResultCode") == State("TargetCode");
     */
    bool targetHit;
    
    /*
     * Holds whether the set of trials has ended
     * The Countdown game is expected to poll for this value regularly
     * When it is sent to the game, the value is reset to false.  
     */
    bool runEnded;
    
    /*
     * Checks the state of the application
     * And returns true if a trial is ready to be run
     *
     * Note: This gives the Countdown server read access to a State variable
     */
    bool isRunning;
    
    /* End of Countdown game objects */
};

#endif // MONGOOSETASK_H

