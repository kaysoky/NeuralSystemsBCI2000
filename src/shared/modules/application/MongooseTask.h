#ifndef MONGOOSETASK_H
#define MONGOOSETASK_H

#include <queue>
#include <vector>

#include "FeedbackTask.h"
#include "OSMutex.h"
#include "mongoose.h"

class MongooseTask : public FeedbackTask {
public:
    MongooseTask();
    virtual ~MongooseTask() {};

    /*
     * Loops infinitely and polls for incoming connections
     * Note: The underlying server can be reinitialized and swapped out at any time
     */
    friend void *MongooseServerThread(void *arg);

    /*
     * Handles the pre-defined set of REST methods
     * Other methods are passed along to Mongoose for default handling
     */
    friend int MongooseServerHandler(struct mg_connection *conn);

protected:
    /*
     * Checks MongooseTask specific parameter settings
     * This should be called within Preflight
     */
    void CheckServerParameters(const SignalProperties& Input) const;

    /*
     * Creates the Mongoose server
     * This should be called within Initialization
     */
    void InitializeServer(const SignalProperties& Input);

    /*
     * Prepares and resets state in preparation for a set of trials
     * This should be called when starting a run
     */
    void PrepareForRun();

    /*
     * Is the client ready to start a trial?
     */
    bool IsClientReady();

    /*
     * Resets trial-specific state in preparation for a single trial
     */
    void PrepareForTrial();
    
    /*
     * Call to indicate that a target was hit
     */
    void IndicateTargetHit();
    
    /*
     * Did the client report a finished trial?
     */
    bool IsClientDone();
    
    /*
     * Call to indicate that a run has ended
     */
    void FinishRun();

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

    /*
     * Provides synchronization for the Mongoose server
     * Any function that touches private variables of this class must first acquire this lock
     * Note: Reading does not require locking
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
     */
    bool isRunning;
};

#endif // MONGOOSETASK_H

