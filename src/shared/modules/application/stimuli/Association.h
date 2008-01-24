////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: Association is a class that associates sets of stimuli with
//   sets of targets.
//   AssociationMap is a map that maps stimulus codes to Associations, and
//   sorts targets according to classification results given over stimulus
//   codes.
//   ClassResult and TargetClassification are auxiliary classes designed as
//   input and output of the AssociationMap's ClassifyTargets() function.
//   ClassResult represents accumulated classification output from signal
//   processing, TargetClassification maps Target pointers to selection
//   likelihood.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef ASSOCIATION_H
#define ASSOCIATION_H

#include "Target.h"
#include "Stimulus.h"
#include "GenericSignal.h"

class Association
{
 public:
  Association();

  Association& SetStimulusDuration( int );
  int StimulusDuration() const;
  Association& SetISIMinDuration( int );
  int ISIMinDuration() const;
  Association& SetISIMaxDuration( int );
  int ISIMaxDuration() const;

  Association& Clear();
  Association& DeleteObjects();

  Association& Add( Stimulus* );
  Association& Remove( Stimulus* );
  bool Contains( Stimulus* ) const;
  bool Intersects( const SetOfStimuli& ) const;

  Association& Add( Target* );
  Association& Remove( Target* );
  bool Contains( Target* ) const;
  bool Intersects( const SetOfTargets& ) const;

  Association& Present();
  Association& Conceal();
  Association& Select();

  SetOfStimuli& Stimuli();
  const SetOfStimuli& Stimuli() const;

  SetOfTargets& Targets();
  const SetOfTargets& Targets() const;

 private:
  SetOfStimuli mStimuli;
  SetOfTargets mTargets;
  int          mStimulusDuration,
               mISIMinDuration,
               mISIMaxDuration;
};

// Classifier output indices are ClassResult[stimulus code][epoch number]
// where "channel" refers to input channel from signal processing,
// and "epoch number" counts the number of inputs for the respective channel
// and stimulus code.
typedef std::map<int, std::vector<GenericSignal> > ClassResult;

// TargetClassification is a map connecting targets with likelihood values.
struct TargetClassification : public std::map<Target*, double>
{
  Target* MostLikelyTarget() const;
};

struct AssociationMap : public std::map<int, Association>
{
  // Given classifier outputs over stimulus codes, channels, and epochs,
  // determine a likelihood value for each target.
  TargetClassification ClassifyTargets( const ClassResult& );

  // Intersection of all sets of stimuli that are associated with a given target.
  SetOfStimuli TargetToStimuli( Target* ) const;

  bool StimuliIntersect( int stimulusCode1, int stimulusCode2 );
  bool TargetsIntersect( int stimulusCode1, int stimulusCode2 );
};

#endif // ASSOCIATION_H

