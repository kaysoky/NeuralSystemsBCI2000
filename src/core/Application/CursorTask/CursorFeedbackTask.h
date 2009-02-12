////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: The CursorFeedback Application's Task filter.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef CURSOR_FEEDBACK_TASK_H
#define CURSOR_FEEDBACK_TASK_H

#include "FeedbackTask.h"
#include "TrialStatistics.h"
#include "Color.h"
#include "TextField.h"
#include "DisplayWindow.h"

#include "FeedbackScene.h"

class CursorFeedbackTask : public FeedbackTask
{
 public:
  CursorFeedbackTask();
  virtual ~CursorFeedbackTask();

 private:
  // Events to be handled by FeedbackTask descendants.
  //  Events triggered by the GenericFilter interface
  virtual void OnPreflight( const SignalProperties& Input ) const;
  virtual void OnInitialize( const SignalProperties& Input );
  virtual void OnStartRun();
  virtual void OnStopRun();
  virtual void OnHalt()                                           {}
  //  Events triggered during the course of a trial
  virtual void OnTrialBegin();
  virtual void OnTrialEnd();
  virtual void OnFeedbackBegin();
  virtual void OnFeedbackEnd();
  //  Dispatching of the input signal.
  //  Each call to GenericSignal::Process() is dispatched to one of these
  //  events, depending on the phase in the sequence.
  //  There, each handler function corresponds to a phase.
  //  If a handler sets the "progress" argument to true, the application's
  //  state will switch to the next phase.
  virtual void DoPreRun(       const GenericSignal&, bool& doProgress );
  virtual void DoPreFeedback(  const GenericSignal&, bool& doProgress );
  virtual void DoFeedback(     const GenericSignal&, bool& doProgress );
  virtual void DoPostFeedback( const GenericSignal&, bool& doProgress );
  virtual void DoITI(          const GenericSignal&, bool& doProgress );

 private:
  void MoveCursorTo( float x, float y, float z );
  void DisplayMessage( const std::string& );

  // Graphic objects
  GUI::DisplayWindow mWindow;
  FeedbackScene*     mpFeedbackScene;
  int                mRenderingQuality;
  TextField*         mpMessage;

  RGBColor mCursorColorFront,
           mCursorColorBack;
  int      mRunCount,
           mTrialCount,
           mCurFeedbackDuration,
           mMaxFeedbackDuration;
  float    mCursorSpeedX,
           mCursorSpeedY,
           mCursorSpeedZ;

  TrialStatistics mTrialStatistics;
};

#endif // CURSOR_FEEDBACK_TASK_H

