////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A stimulus consisting of a text field.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "TextStimulus.h"

using namespace std;
using namespace GUI;

TextStimulus::TextStimulus( GraphDisplay& display )
: TextField( display, TextStimulusZOrder ),
  mIntensifiedColor( RGBColor::White )
{
}

TextStimulus::~TextStimulus()
{
}

TextStimulus&
TextStimulus::SetIntensifiedColor( RGBColor c )
{
  mIntensifiedColor = c;
  Change();
  return *this;
}

RGBColor
TextStimulus::IntensifiedColor() const
{
  return mIntensifiedColor;
}

void
TextStimulus::OnPaint( const DrawContext& dc )
{
  bool doPaint = false;
  RGBColor backgroundColor = RGBColor::Black,
           textColor = RGBColor::Black;
  if( BeingPresented() )
    switch( PresentationMode() )
    {
      case ShowHide:
        doPaint = true;
        backgroundColor = Color();
        textColor = TextColor();
        break;

      case Intensify:
        doPaint = true;
        backgroundColor = Color();
        textColor = IntensifiedColor();
        break;

      case Grayscale:
        doPaint = true;
        backgroundColor = Color().ToGray();
        textColor = TextColor().ToGray();
        break;

      case Invert:
        doPaint = true;
        backgroundColor = TextColor();
        textColor = Color();
        break;

      case Dim:
        doPaint = true;
        backgroundColor = DimFactor() * Color();
        textColor = DimFactor() * TextColor();
        break;
    }
  else // not being presented
    switch( PresentationMode() )
    {
      case ShowHide:
        doPaint = false;
        break;

      case Intensify:
      case Grayscale:
      case Invert:
      case Dim:
        doPaint = true;
        backgroundColor = Color();
        textColor = TextColor();
        break;
    }
  if( doPaint )
    TextField::DoPaint( dc, textColor, backgroundColor );
}

