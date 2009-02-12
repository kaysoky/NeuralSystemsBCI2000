////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A virtual base class for stimuli that are also graphic objects.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef VISUAL_STIMULUS_H
#define VISUAL_STIMULUS_H

#include "Stimulus.h"

class VisualStimulus : public Stimulus
{
 public:
  enum Mode
  {
    ShowHide,
    Intensify,
    Grayscale,
    Invert,
    Dim,
  };

  VisualStimulus();
  virtual ~VisualStimulus();
  // Properties
  VisualStimulus& SetPresentationMode( Mode m );
  Mode PresentationMode() const;
  VisualStimulus& SetDimFactor( float );
  float DimFactor() const;

 protected:
  // Stimulus event handlers
  virtual void OnPresent();
  virtual void OnConceal();

 protected:
  bool BeingPresented() const;

 private:
  bool    mBeingPresented;
  Mode    mPresentationMode;
  float   mDimFactor;
};

#endif // VISUAL_STIMULUS_H

