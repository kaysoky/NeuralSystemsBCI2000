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
  m_pLabel = NULL;
  m_asText = "";
  m_bkgdColor = clYellow;
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
  if (m_pLabel != NULL) delete m_pLabel;
  m_pLabel = NULL;
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
  pClonedElement->SetCaptionAttributes(m_bkgdColor, m_asText, m_textColor, m_uTextHeight);
  pClonedElement->SetCoordsRect(this->GetCoordsRect());

  return pClonedElement;
}  // GetClone


// **************************************************************************
// Function:   Render
// Purpose:    This function renders this element onto the specified form
// Parameters: form     - pointer to the form that will hold the element
//             destRect - part of the form the element will be rendered into
// Returns:    N/A
// **************************************************************************
void UsrElementCaption::Render(TForm * form, const TRect & destRect)
{
  // create the label, if not already exists
  if ((m_asText != "") && (m_pLabel == NULL))
    m_pLabel = new TLabel(Application);
    
  if (m_pLabel != NULL)
  {
    float fScaleY = ((float)destRect.Height()) / 65536.0f;
    int iScaledTextSize = (int)((float)m_uTextHeight * 655.36f * fScaleY);
    form->Canvas->Font->Height = -iScaledTextSize;
    form->Canvas->Font->Name = "Arial";
    form->Canvas->Font->Style = TFontStyles() << fsBold;
  
    // set the text's properties
    m_pLabel->Parent = form;
    m_pLabel->Caption = m_asText;
    m_pLabel->Color = m_bkgdColor;
    m_pLabel->Font->Color = m_textColor;
    m_pLabel->Visible      = true;
    m_pLabel->Enabled      = true;
    m_pLabel->Layout       = tlBottom;
    m_pLabel->Transparent  = true;
    m_pLabel->Font->Height = -iScaledTextSize;
    m_pLabel->Font->Name   = "Arial";
    m_pLabel->Font->Style  = TFontStyles() << fsBold;
    const TRect scaledRect(this->GetScaledCoordsRect(destRect));
    const int iScaledTextPosX = abs((scaledRect.Left + scaledRect.Right) / 2 - form->Canvas->TextWidth(m_pLabel->Caption) / 2);
    const int iScaledTextPosY = (scaledRect.Bottom + scaledRect.Top) / 2 - m_pLabel->Height - scaledRect.Height() / 2;
    m_pLabel->Left = iScaledTextPosX;
    m_pLabel->Top  = iScaledTextPosY;
  }
} // Render


// **************************************************************************
// Function:   Hide
// Purpose:    This function hides this element
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void UsrElementCaption::Hide(void)
{
  // hide the text, if it exists
  if (m_pLabel != NULL)
    m_pLabel->Visible = false;
}  // Hide


// **************************************************************************
// Function:   SetCaptionBkgdColor
// Purpose:    This function sets the text color of this element
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void UsrElementCaption::SetCaptionBkgdColor(TColor bkgdColor)
{
  // set the color of the label
  m_bkgdColor = bkgdColor;
}// SetColor


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
void UsrElementCaption::SetCaptionAttributes(TColor bkgdColor, const AnsiString & asText,
                                      TColor textColor, const unsigned int & uTextHeight)
{
  SetCaptionBkgdColor(bkgdColor);
  SetCaptionText(asText);
  SetCaptionTextColor(textColor);
  SetCaptionTextHeight(uTextHeight);
} // SetAttributes


