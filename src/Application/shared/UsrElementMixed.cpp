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
  pClonedElement->SetCaptionAttributes(this->GetCaptionBkgdColor(), this->GetCaptionText(),
                                this->GetCaptionTextColor(), this->GetCaptionTextHeight());
  pClonedElement->SetIconFileName(this->GetIconFileName());
  pClonedElement->SetAudioFileName(this->GetAudioFileName());

  return pClonedElement;
}


// **************************************************************************
// Function:   Render
// Purpose:    This function renders the element
// Parameters: form     - pointer to the form that will hold the element
//             destRect - part of the form the element will be rendered into
// Returns:    N/A
// **************************************************************************
void UsrElementMixed::Render(TForm * form, const TRect & destRect)
{
  UsrElementAudio::Render(form, destRect);
  UsrElementCaption::Render(form, destRect);
  UsrElementVideo::Render(form, destRect);
}   // Render


// **************************************************************************
// Function:   Hide
// Purpose:    This function hides this element
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void UsrElementMixed::Hide(void)
{
  UsrElementCaption::Hide();
  UsrElementVideo::Hide();
  UsrElementAudio::Hide();
}

