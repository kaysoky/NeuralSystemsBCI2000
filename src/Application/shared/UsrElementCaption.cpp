/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
#pragma hdrstop
#pragma package(smart_init)
#include "UsrElementCaption.h"

// **************************************************************************
// Function:   UsrElementCaption
// Purpose:    This is the constructor of the UsrElementCaption class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
UsrElementCaption::UsrElementCaption(const unsigned int & uElementID) : UsrElement(uElementID)
{
  m_asText = "";
  m_textColor = clBlack;
  m_uTextHeight = 8;
}  // UsrElementCaption


// **************************************************************************
// Function:   ~UsrElementCaption
// Purpose:    This is the destructor of the UsrElementCaption class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
UsrElementCaption::~UsrElementCaption()
{
}// ~UsrElementCaption


// **************************************************************************
// Function:   GetClone
// Purpose:    This function clones a particular element by "deep-copying" the old one
// Parameters: N/A
// Returns:    pointer to the cloned element
// **************************************************************************
UsrElement * UsrElementCaption::GetClone(void) const
{
  UsrElementCaption * pClonedElement = new UsrElementCaption(this->GetID());
  pClonedElement->SetCaptionAttributes(m_asText, m_textColor, m_uTextHeight);
  pClonedElement->SetCoordsRect(this->GetCoordsRect());

  return pClonedElement;
}  // GetClone


// **************************************************************************
// Function:   Render
// Purpose:    This function renders the element onto a canvas
// Parameters: canvas   - pointer to the canvas that the element will be
//                        drawn upon
//             destRect - part of the canvas the element will be rendered into
// Returns:    N/A
// **************************************************************************
void UsrElementCaption::Render( TCanvas& ioCanvas, const TRect& inDestRect ) const
{
  if( Visible() )
  {
    float fScaleY = ((float)inDestRect.Height()) / 65536.0f;
    int iScaledTextSize = (int)((float)m_uTextHeight * 655.36f * fScaleY);
    // set the text's properties
    ioCanvas.Font->Height = -iScaledTextSize;
    ioCanvas.Font->Name = "Arial";
    ioCanvas.Font->Style = TFontStyles() << fsBold;
    ioCanvas.Font->Color = m_textColor;
    ioCanvas.Brush->Style = bsClear;

    const TRect scaledRect(this->GetScaledCoordsRect(inDestRect));
    const int iScaledTextPosX = abs((scaledRect.Left + scaledRect.Right) / 2 - ioCanvas.TextWidth(m_asText) / 2);
    const int iScaledTextPosY = (scaledRect.Bottom + scaledRect.Top) / 2 - ioCanvas.TextHeight(m_asText) - scaledRect.Height() / 2;
    ioCanvas.TextOut( iScaledTextPosX, iScaledTextPosY, m_asText );
  }
} // Render


// **************************************************************************
// Function:   SetCaptionTextColor
// Purpose:    This function sets the text color of this element
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void UsrElementCaption::SetCaptionTextColor(TColor textColor)
{
  // set the text color
  m_textColor = textColor;
} // SetTextColor


// **************************************************************************
// Function:   SetCaptionText
// Purpose:    This function sets the text of this element
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void UsrElementCaption::SetCaptionText(const AnsiString & asText)
{
  // set the text color
  m_asText = asText;
} // SetText


// **************************************************************************
// Function:   SetCaptionTextHeight
// Purpose:    This function sets the text height of this element
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void UsrElementCaption::SetCaptionTextHeight(const unsigned int & uTextHeight)
{
  // set the text height
  m_uTextHeight = uTextHeight;
} // SetTextHeight


// **************************************************************************
// Function:   SetAttributes
// Purpose:    This function sets the text height of this element
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void UsrElementCaption::SetCaptionAttributes(const AnsiString & asText,
                                      TColor textColor, const unsigned int & uTextHeight)
{
  SetCaptionText(asText);
  SetCaptionTextColor(textColor);
  SetCaptionTextHeight(uTextHeight);
} // SetAttributes




