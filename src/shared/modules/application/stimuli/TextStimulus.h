////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A stimulus consisting of a text field.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef TEXT_STIMULUS_H
#define TEXT_STIMULUS_H

#include "VisualStimulus.h"
#include "TextField.h"
#include "Color.h"

class TextStimulus : public VisualStimulus, public TextField
{
 public:
  TextStimulus( GUI::GraphDisplay& display );
  virtual ~TextStimulus();
  // Properties
  TextStimulus& SetIntensifiedColor( RGBColor );
  RGBColor IntensifiedColor() const;

 protected:
  // GraphObject event handlers
  virtual void OnPaint( const GUI::DrawContext& );

 private:
  RGBColor mIntensifiedColor;
};

#endif // TEXT_STIMULUS_H

