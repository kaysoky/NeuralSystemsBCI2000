////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A base class for application modules that provide feedback in a
//   trial-based paradigm.
//   Inheriting from ApplicationBase, descendants of FeedbackTask have access
//   to the AppLog, AppLog.File and AppLog.Screen streams declared in
//   ApplicationBase.
//
//   This class performs sequencing, and dispatches GenericFilter::Process()
//   calls to its virtual member functions, depending on the current trial
//   phase. Child classes (descendants) of FeedbackTask implement event
//   handlers by overriding its virtual functions.
//
//   Sequence of events         Typical application behavior
//
//   OnPreflight
//   OnInitialize
//   OnStartRun                 display initial message
//   DoPreRun*
//   Loop {
//    OnTrialBegin              display target
//    DoPreFeedback*
//    OnFeedbackBegin           show cursor
//    DoFeedback*               update cursor position
//    OnFeedbackEnd             hide cursor, mark target as hit
//    DoPostFeedback*
//    OnTrialEnd                hide targets
//    DoITI*
//   }
//   OnStopRun                  display final message
//   OnHalt
//
//   Events marked with * will occur multiple times in a row.
//   Progress from one state to the next will occur according to the sequencing
//   parameters, or if requested by a handler via its doProgress output argument.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef FEEDBACK_TASK_H
#define FEEDBACK_TASK_H

#include "ApplicationBase.h"
#include "BlockRandSeq.h"

class FeedbackTask : public ApplicationBase
{
 private:
  enum TaskPhases
  {
    none,
    preRun,
    preFeedback,
    feedback,
    postFeedback,
    ITI,
    postRun,
  };
  // Events to be handled by FeedbackTask descendants.
  //  Events triggered by the GenericFilter interface
  virtual void OnPreflight( const SignalProperties& Input ) const {}
  virtual void OnInitialize( const SignalProperties& Input )      {}
  virtual void OnStartRun()                                       {}
  virtual void OnStopRun()                                        {}
  virtual void OnHalt()                                           {}
  //  Events triggered during the course of a trial
  virtual void OnTrialBegin()                                     {}
  virtual void OnTrialEnd()                                       {}
  virtual void OnFeedbackBegin()                                  {}
  virtual void OnFeedbackEnd()                                    {}
  //  Dispatching of the input signal.
  //  Each call to GenericSignal::Process() is dispatched to one of these
  //  events, depending on the phase in the sequence.
  //  There, each handler function corresponds to a phase.
  //  If a handler sets the "progress" argument to true, the application's
  //  state will switch to the next phase independently of the phases' pre-set
  //  durations.
  virtual void DoPreRun(       const GenericSignal&, bool& doProgress ) {}
  virtual void DoPreFeedback(  const GenericSignal&, bool& doProgress ) {}
  virtual void DoFeedback(     const GenericSignal&, bool& doProgress ) {}
  virtual void DoPostFeedback( const GenericSignal&, bool& doProgress ) {}
  virtual void DoITI(          const GenericSignal&, bool& doProgress ) {}

 protected:
  FeedbackTask( const GUI::GraphDisplay* = NULL );

 public:
  virtual ~FeedbackTask();

  // Implementation of the GenericFilter interface.
  virtual void Preflight(  const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize( const SignalProperties&, const SignalProperties& );
  virtual void Process(    const GenericSignal&,    GenericSignal& );
  virtual void StartRun();
  virtual void StopRun();
  virtual void Halt();

 private:
  int       mPhase,
            mBlocksInPhase;
  long long mBlocksInRun;

  int       mPreRunDuration,
            mPreFeedbackDuration,
            mFeedbackDuration,
            mPostFeedbackDuration,
            mITIDuration,
            mNumPresentations,
            mNumTotalPresentations;
  long long mMinRunLength;

  BlockRandSeq    mBlockRandSeq;
};

#endif // FEEDBACK_TASK_H

