#pragma hdrstop
#pragma package(smart_init)
#include "UsrElementVideo.h"


// **************************************************************************
// Function:   UsrElementVideo
// Purpose:    This is the constructor of the UsrElementVideo class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
UsrElementVideo::UsrElementVideo(const unsigned int & uElementID) : UsrElement(uElementID)
{
  m_pIcon = NULL;
  m_asIconFile = "";
}


// **************************************************************************
// Function:   ~UsrElementVideo
// Purpose:    This is the destructor of the UsrElementVideo class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
UsrElementVideo::~UsrElementVideo()
{
 if (m_pIcon) delete m_pIcon;
 m_pIcon = NULL;
}


// **************************************************************************
// Function:   GetClone
// Purpose:    This function clones a particular element by "deep-copying" the old one
// Parameters: N/A
// Returns:    pointer to the cloned element
// **************************************************************************
UsrElement * UsrElementVideo::GetClone(void) const
{
  UsrElementVideo * pClonedElement = new UsrElementVideo(this->GetID());
  pClonedElement->SetIconFileName(this->GetIconFileName());
  pClonedElement->SetCoordsRect(this->GetCoordsRect());

  return pClonedElement;
}


// **************************************************************************
// Function:   Render
// Purpose:    This function renders this element onto the specified form
// Parameters: form     - pointer to the form that will hold the element
//             destRect - part of the form the element will be rendered into
// Returns:    N/A
// **************************************************************************
void UsrElementVideo::Render(TForm * form, const TRect & destRect)
{
  // set the icon's properties
  if (m_pIcon)
  {
    m_pIcon->Parent = form;
    m_pIcon->Visible = true;
    m_pIcon->Enabled = true;
    m_pIcon->Stretch = true;
    const TRect scaledRect(this->GetScaledCoordsRect(destRect));
    m_pIcon->Left    = scaledRect.Left + 1;
    m_pIcon->Top     = scaledRect.Top + 1;
    m_pIcon->Width   = scaledRect.Width() - 2;
    m_pIcon->Height  = scaledRect.Height() - 2;
  }
}   // Render


// **************************************************************************
// Function:   Hide
// Purpose:    This function hides this element
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void UsrElementVideo::Hide(void)
{
  // hide the icon, if it exists
  if (m_pIcon) m_pIcon->Visible = false;
}


// **************************************************************************
// Function:   SetIconFileName
// Purpose:    This function sets the icon file name
// Parameters: asIconFile - file name
// Returns:    N/A
// **************************************************************************
void UsrElementVideo::SetIconFileName(const AnsiString & asIconFile)
{
  m_asIconFile = asIconFile;
  // create the icon, if not already exists
  if ((m_asIconFile != "") && (m_pIcon == NULL))
    m_pIcon = new TImage(Application);
  // set the icon's properties
  if (m_pIcon)
  {
    bool bError(false);
    try
    {
      m_pIcon->Picture->LoadFromFile(m_asIconFile);
    }
    catch( EInOutError & )
    {
      bError = true;
    }
    catch( EInvalidGraphic & )
    {
      bError = true;
    }
    if (bError == true)
    {
      delete m_pIcon;
      m_pIcon = NULL;
    }
    // change coordinates of the element according to aspect ration
    float fAspectRatio(0.0f);
    if (m_pIcon->Picture->Width != 0)
      fAspectRatio = (float)m_pIcon->Picture->Height / (float)m_pIcon->Picture->Width;
    SetCoordsRect(GetCoordsRect().Width(), fAspectRatio, true);
  }
} // SetIconFileName


// **************************************************************************
// Function:   GetIconFileName
// Purpose:    This function returns the icon file name
// Parameters: N/A
// Returns:    icon file name
// **************************************************************************
const AnsiString & UsrElementVideo::GetIconFileName(void) const
{
  return m_asIconFile;
} // GetIconFileName

