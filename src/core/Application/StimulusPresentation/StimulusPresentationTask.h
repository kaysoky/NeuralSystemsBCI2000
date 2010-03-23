////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: jhizver@wadsworth.org, schalk@wadsworth.org,
//   juergen.mellinger@uni-tuebingen.de
// Description: The task filter for a stimulus presentation task.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef STIMULUS_PRESENTATION_TASK_H
#define STIMULUS_PRESENTATION_TASK_H

#include "StimulusTask.h"
#include <vector>

namespace SequenceTypes
{
  enum
  {
    Deterministic = 0,
    Random,
  };
}

class StimulusPresentationTask : public StimulusTask
{
 public:
  StimulusPresentationTask();
  ~StimulusPresentationTask();

 protected:
  // StimulusTask events
  virtual void    OnPreflight( const SignalProperties& Input ) const;
  virtual void    OnInitialize( const SignalProperties& Input );
  virtual void    OnStartRun();
  virtual void    OnStopRun();
  virtual void    OnPreSequence();
  virtual void    OnSequenceBegin();
  virtual void    OnPostRun();
  virtual Target* OnClassResult( const ClassResult& );
  virtual int     OnNextStimulusCode();

  virtual void DoPreSequence(  const GenericSignal&, bool& doProgress );
  virtual void DoPostSequence( const GenericSignal&, bool& doProgress );

 private:
  void DetermineAttendedTarget();
  ParamRef StimulusProperty( const ParamRef& inMatrixParam,
                             int inColIndex,
                             const std::string& inPropertyName ) const;

  // Configuration parameters.
  int mNumberOfSequences,
      mSequenceType;

  // Internal state.
  int mBlockCount;

  std::vector<int> mToBeCopied;
  std::vector<int>::const_iterator mToBeCopiedPos;

  std::vector<int> mSequence;
  std::vector<int>::const_iterator mSequencePos;

  // Sets of stimuli
  SetOfStimuli mStimuli;
  Association  mFocusAnnouncement,
               mResultAnnouncement;
  // Set of all existing targets.
  SetOfTargets mTargets;
};


#endif // P3_SPELLER_TASK_H