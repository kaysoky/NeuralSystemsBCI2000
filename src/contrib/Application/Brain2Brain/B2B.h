#ifndef B2B_H
#define B2B_H

#include <queue>
#include <vector>

#include "FeedbackTask.h"
#include "TrialStatistics.h"
#include "Color.h"
#include "TextField.h"
#include "ApplicationWindow.h"

#include "DFBuildScene.h"

// For interfacing with the Countdown Game
extern "C" {
  #include "mongoose.h"
}
#include "OSMutex.h"

class DynamicFeedbackTask : public FeedbackTask {
public:
    DynamicFeedbackTask();
    virtual ~DynamicFeedbackTask();
    
    /*
     * Checks the state of the application
     * And returns true if a trial is ready to be run
     *
     * Note: This gives the Countdown server read access to a State variable
     */
    bool isRunning();
	
    /* Objects associated with the Countdown game */
    // Note: Since Mongoose is written in C, and needs to access these variables
    //       I've set them up as public variables.
    
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
     * Any function that touches the following objects must first acquire this lock
     * Note: Reading does not require locking
     */
    OSMutex *server_lock;
    
    /*
     * Mongoose server
     * Serves up the Countdown Game files
     *   and handles the various communication REST functions
     */
	struct mg_server *server;
    
    /*
     * Holds a random list of trials to pass onto the Countdown game
     * If empty, then just choose a random trial type instead
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
    
    /* End of Countdown game objects */

private:
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

private:
    void MoveCursorTo(float x, float y, float z);
    void DisplayMessage(const std::string&);
    void DisplayScore(const std::string&);

    // Graphic objects
    ApplicationWindow& mrWindow;
    DFBuildScene*      mpFeedbackScene;
    TextField*         mpMessage;
    TextField*         mpMessage2;

    RGBColor mCursorColor;

    int mRunCount,
        mTrialCount,
        mCurFeedbackDuration,
        mMaxFeedbackDuration;

    float mCursorSpeedX,
          mCursorSpeedY,
          mCursorSpeedZ,
          mScore;

    std::vector<int> mVisualCatchTrials;

    bool mVisualFeedback,
         mIsVisualCatchTrial;

    TrialStatistics mTrialStatistics;
};

#endif // CURSOR_FEEDBACK_TASK_H

