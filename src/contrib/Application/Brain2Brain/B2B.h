#ifndef B2B_H
#define B2B_H

#include "FeedbackTask.h"
#include "TrialStatistics.h"
#include "Color.h"
#include "TextField.h"
#include "ApplicationWindow.h"

#include "DFBuildScene.h"
#include "SockStream.h"

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

    client_tcpsocket mSocket;
    std::string mConnectorAddress;
};

#endif // CURSOR_FEEDBACK_TASK_H

