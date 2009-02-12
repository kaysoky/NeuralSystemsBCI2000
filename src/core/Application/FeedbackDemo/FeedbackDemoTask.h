////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A demo feedback task.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef FEEDBACK_DEMO_TASK_H
#define FEEDBACK_DEMO_TASK_H

#include "FeedbackTask.h"
#include "TrialStatistics.h"

class FeedbackDemoTask : public FeedbackTask
{
 public:
  FeedbackDemoTask();
  virtual ~FeedbackDemoTask();

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
  int   mRunCount,
        mTrialCount;
  float mCursorPosX,
        mCursorPosY,
        mCursorSpeedX,
        mCursorSpeedY;

  TrialStatistics mTrialStatistics;

  class TForm*  mpForm;
  class TLabel* mpLabel;
  class TShape* mpCursor,
              * mpTarget;
};

#endif // FEEDBACK_DEMO_TASK_H
