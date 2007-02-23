/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
#pragma hdrstop
#pragma package(smart_init)
#include "PCHIncludes.h"
#include "UsrElement.h"
#include "UBCIError.h"
#include "UCursor.h"

// **************************************************************************
// Function:   UsrElement
// Purpose:    This is the constructor of the UsrElement class
// Parameters: uElementID - ID of the user element
// Returns:    N/A
// **************************************************************************
UsrElement::UsrElement(const unsigned int & uElementID)
: m_uElementID(uElementID),
  m_rectElementCoords(0, 0, 0, 0),
  m_bVisible( false )
{
}// UsrElement


// **************************************************************************
// Function:   ~UsrElement
// Purpose:    This is the destructor of the UsrElement class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
UsrElement::~UsrElement()
{
} // ~UsrElement


// **************************************************************************
// Function:   SelectionSucceeded
// Purpose:    This function detects whether there is collision with this particular element
// Parameters: cursor - pointer to the cursor
// Returns:    true if the element has been selected , false otherwise
// **************************************************************************
const bool UsrElement::SelectionSucceeded(CURSOR * pCursor) const
{
  // in coordinates 0..65535, determine the cursor's "hot spot"
  const int iCursorPosX = pCursor->Left + pCursor->Width  / 2;
  const int iCursorPosY = pCursor->Top  + pCursor->Height / 2;

  // has the cursor hit the target ?
  // in this case, selection is purely based on coordinates; could also be based on dwell time
  bool bIsSelected(false);
  if ((iCursorPosX > m_rectElementCoords.Left) && (iCursorPosX <= m_rectElementCoords.Left + m_rectElementCoords.Width()))
    if ((iCursorPosY > m_rectElementCoords.Top) && (iCursorPosY <= m_rectElementCoords.Top + m_rectElementCoords.Height()))
      bIsSelected = true;

  return bIsSelected;
} // SelectionSucceeded


// **************************************************************************
// Function:   GetScaledCoordsRect
// Purpose:    This function scales the coordinates rectangle of an element
//             according to the required destination rectangle
// Parameters: destRect - destination rectangle
// Returns:    returns a new scaled coordinates rectangle
// **************************************************************************
const TRect UsrElement::GetScaledCoordsRect(const TRect & destRect) const
{
  float fScaleX = ((float)destRect.Width())  / 65536.0f;
  float fScaleY = ((float)destRect.Height()) / 65536.0f;

  const int iScaledTop    =(int)((float)m_rectElementCoords.Top    * fScaleY + (float)destRect.Top);
  const int iScaledBottom =(int)((float)m_rectElementCoords.Bottom * fScaleY + (float)destRect.Top);
  const int iScaledLeft   =(int)((float)m_rectElementCoords.Left   * fScaleX + (float)destRect.Left);
  const int iScaledRight  =(int)((float)m_rectElementCoords.Right  * fScaleX + (float)destRect.Left);

  const TRect scaledCoordsRect(iScaledLeft, iScaledTop, iScaledRight, iScaledBottom);

  return scaledCoordsRect;
} // GetScaledCoordsRect


// **************************************************************************
// Function:   SetCoordsRect
// Purpose:    This function calculates the coordinates rect according to specified
//             width in percent of screen width and aspect ratio
// Parameters:
// Returns:
// **************************************************************************
void UsrElement::SetCoordsRect(const float & fWidthInPercentOfScreen, const float & fAspectRatio,
                               const bool bUseExistingWidth)
{
  int iWidth(0);
  if (bUseExistingWidth == false)
    iWidth = (int)(fWidthInPercentOfScreen * 655.36f + 0.5f);
  else
    iWidth = m_rectElementCoords.Width();
  const int iHeight((float)iWidth * fAspectRatio + 0.5f);
  const int iRight(65536.0f / 2.0f + (float)iWidth / 2.0f + 0.5f);
  const int iBottom(65536.0f / 2.0f + (float)iHeight / 2.0f + 0.5f);
  const int iLeft(iRight - iWidth);
  const int iTop(iBottom - iHeight);
  const TRect coordsRect(iLeft, iTop, iRight, iBottom);
  SetCoordsRect(coordsRect);
}// SetCoordsRect







