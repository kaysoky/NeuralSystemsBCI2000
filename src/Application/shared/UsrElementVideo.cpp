/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
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
  // pClonedElement->SetIconFileName(this->GetIconFileName());          // DO NOT READ FROM DISC WHEN CLONING!!!
  pClonedElement->CloneIcon(this);
  pClonedElement->SetCoordsRect(this->GetCoordsRect());

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
void UsrElementVideo::Render( TCanvas& ioCanvas, const TRect& inDestRect ) const
{
  if( Visible() && m_pIcon != NULL )
  {
    TRect scaledRect( this->GetScaledCoordsRect( inDestRect ) );
    ioCanvas.StretchDraw( scaledRect, m_pIcon->Graphic );
  }
}   // Render


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
    m_pIcon = new TPicture;
  // set the icon's properties
  if (m_pIcon)
  {
    bool bError(false);
    try
    {
      m_pIcon->LoadFromFile(m_asIconFile);
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
    if (m_pIcon->Width != 0)
      fAspectRatio = (float)m_pIcon->Height / (float)m_pIcon->Width;
    SetCoordsRect(GetCoordsRect().Width(), fAspectRatio, true);
  }
} // SetIconFileName


// **************************************************************************
// Function:   CloneIcon
// Purpose:    This function clones the icon from another UsrElementVideo
// Parameters: UsrElementVideo * - ptr to another UsrElementVideo
// Returns:    N/A
// **************************************************************************
void UsrElementVideo::CloneIcon(const UsrElementVideo *src)
{
  m_asIconFile = src->GetIconFileName();
  // create the icon, if not already exists
  if ((m_asIconFile != "") && (m_pIcon == NULL))
     m_pIcon= new TPicture;
  // set the icon's properties
  if (m_pIcon)
  {
    try
    {
      m_pIcon->Assign(src->m_pIcon);
    }
    catch(...)
    {
      delete m_pIcon;
      m_pIcon = NULL;
    }
    // change coordinates of the element according to aspect ration
    float fAspectRatio(0.0f);
    if (m_pIcon)
       {
       if (m_pIcon->Width != 0)
         fAspectRatio = (float)m_pIcon->Height / (float)m_pIcon->Width;
       SetCoordsRect(GetCoordsRect().Width(), fAspectRatio, true);
       }
  }
} // CloneIcon


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


// **************************************************************************
// Function:   GetIconPicture
// Purpose:    This function returns the icon picture (i.e., TPicture property)
// Parameters: N/A
// Returns:    pointer to this icon's TPicture
// **************************************************************************
const TPicture * UsrElementVideo::GetIconPicture(void) const
{
  return m_pIcon;
} // GetIconPicture


