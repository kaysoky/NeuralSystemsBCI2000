/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
#pragma hdrstop
#pragma package(smart_init)
#include "UsrElementMixed.h"

// **************************************************************************
// Function:   UsrElementMixed
// Purpose:    This is the constructor of the UsrElementMixed class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
UsrElementMixed::UsrElementMixed(const unsigned int & uElementID)
                            : UsrElement(uElementID),
                              UsrElementCaption(uElementID),
                              UsrElementVideo(uElementID),
                              UsrElementAudio(uElementID)
{
} // UsrElementMixed


// **************************************************************************
// Function:   ~UsrElementMixed
// Purpose:    This is the destructor of the UsrElementMixed class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
UsrElementMixed::~UsrElementMixed()
{
} // ~UsrElementMixed


// **************************************************************************
// Function:   GetClone
// Purpose:    This function clones a particular element by "deep-copying" the old one
// Parameters: N/A
// Returns:    pointer to the cloned element
// **************************************************************************
UsrElement * UsrElementMixed::GetClone(void) const
{
  UsrElementMixed * pClonedElement = new UsrElementMixed(this->GetID());
  pClonedElement->SetCoordsRect(this->GetCoordsRect());
  pClonedElement->SetCaptionAttributes(this->GetCaptionText(),
                                this->GetCaptionTextColor(), this->GetCaptionTextHeight());
  // pClonedElement->SetIconFileName(this->GetIconFileName());    // DO NOT READ FROM DISC WHEN CLONING!!!
  pClonedElement->CloneIcon(this);
  // pClonedElement->SetAudioFileName(this->GetAudioFileName());  // DO NOT READ FROM DISC WHEN CLONING!!!
  pClonedElement->CloneAudio(this);

  return pClonedElement;
}


// **************************************************************************
// Function:   Render
// Purpose:    This function renders the element onto a canvas
// Parameters: canvas   - pointer to the canvas that the element will be
//                        drawn upon
//             destRect - part of the canvas the element will be rendered into
// Returns:    N/A
// **************************************************************************
void UsrElementMixed::Render( TCanvas& ioCanvas, const TRect& inDestRect ) const
{
  UsrElementAudio::Render(ioCanvas, inDestRect);
  UsrElementCaption::Render(ioCanvas, inDestRect);
  UsrElementVideo::Render(ioCanvas, inDestRect);
}   // Render


// **************************************************************************
// Function:   Show
// Purpose:    This function makes the element visible
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void UsrElementMixed::Show()
{
  UsrElementAudio::Show();
  UsrElementCaption::Show();
  UsrElementVideo::Show();
}   // Show


// **************************************************************************
// Function:   Hide
// Purpose:    This function hides this element
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void UsrElementMixed::Hide(void)
{
  UsrElementAudio::Hide();
  UsrElementCaption::Hide();
  UsrElementVideo::Hide();
}   // Hide


