#ifndef B2B_H
#define B2B_H

#include "FeedbackTask.h"
#include "TrialStatistics.h"
#include "Color.h"
#include "TextField.h"
#include "ApplicationWindow.h"

#include "DFBuildScene.h"

// For interfacing with the Countdown Game
#include "mongoose.h"
#include "OSMutex.h"

class DynamicFeedbackTask : public FeedbackTask {
public:
    DynamicFeedbackTask();
    virtual ~DynamicFeedbackTask();

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
          mScore, //Score
          mScoreCount; //Score counter


    std::vector<int> mVisualCatchTrials;

    bool mVisualFeedback,
         mIsVisualCatchTrial;

    TrialStatistics mTrialStatistics;
	
    // Objects and functions associated with the Countdown game
    
    /*
     * Loops infinitely and polls for incoming connections
     */
    void *CountdownServerThread(void *);
    
    /*
     * Handles the pre-defined set of REST methods
     * Other methods are passed along to Mongoose for default handling
     */
    int CountdownServerHandler(struct mg_connection *);
    
    // Any function that touches the following objects must first acquire this lock
    // Note: Reading does not require locking
    OSMutex server_lock;
	struct mg_server *server;
    enum TrialState {
        START_TRIAL, 
        STOP_TRIAL, 
        CONTINUE
    };
    TrialState lastClientPost;
    bool targetHit;
};

#endif // CURSOR_FEEDBACK_TASK_H

