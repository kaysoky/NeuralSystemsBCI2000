/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
#pragma hdrstop
#pragma package(smart_init)
#include <stdio.h>
#include "PCHIncludes.h"
#include "UsrEnv.h"
#include "UsrEnvAlgorithmP3AV.h"
#include "UParameter.h"
#include "Task.h"
#include "Localization.h"

// **************************************************************************
// Function:   UsrEnv
// Purpose:  Constructor
// Parameters:  asTaskName - the name of a task
// Returns:   void
//
// **************************************************************************
UsrEnv::UsrEnv(const AnsiString & asTaskName, UsrEnvAlgorithm * pAlgorithm)
{
  m_pUsrElementColl = NULL;
  m_pActiveUsrElementColl = NULL;
  m_pAlgorithm = NULL;
  m_pAlgorithm = pAlgorithm;

  // create a new form
  m_pForm = new TForm(static_cast<TComponent*>(NULL));
  m_pForm->Top     = 50;
  m_pForm->Left    = 50;
  m_pForm->Width   = 300;
  m_pForm->Height  = 300;
  m_pForm->Caption = asTaskName;
  m_pForm->Color   = clDkGray;

  // disable automatic scrollbars
  m_pForm->AutoScroll = false;
  m_pForm->BorderIcons >> biSystemMenu;
  m_pForm->BorderStyle = bsNone;

  // create a new 
  m_pMessage = new TLabel(static_cast<TComponent*>(NULL));
  m_pMessage->Parent  = m_pForm;
  m_pMessage->Visible = false;
} // UsrEnv


// **************************************************************************
// Function:   ~UsrEnv
// Purpose:    Destructor
// Parameters:   void
// Returns:      void
//
// **************************************************************************
UsrEnv::~UsrEnv()
{
  if (m_pMessage) delete m_pMessage;
  if (m_pUsrElementColl) delete m_pUsrElementColl;
  if (m_pActiveUsrElementColl != NULL) delete m_pActiveUsrElementColl;
  if (m_pAlgorithm != NULL) delete m_pAlgorithm;
  if (m_pForm) m_pForm->Close(); delete m_pForm;

  m_pMessage = NULL;
  m_pUsrElementColl = NULL;
  m_pActiveUsrElementColl = NULL;
  m_pForm = NULL;
  m_pAlgorithm = NULL;
} // ~UsrEnv


void UsrEnv::Initialize(TTask * pTask, TApplication * pApplication, const int & iTop, const int & iLeft,
                        const int & iWidth,const int & iHeight, TColor color)
{
#if 0 // Do we actually want to flash the window on every Initialize()?
  if (m_pForm != NULL)
    if (m_pForm->Visible == true)
      m_pForm->Close();
#endif

  // Translate any strings visible on the form.
  ApplyLocalizations( m_pForm );
      
  // Initialize usr element collection
  InitializeAlgorithm(pTask , pApplication);
  
  // set the window position, size, and background color
  SetWindowSize(iTop, iLeft, iWidth, iHeight, color);

  DisplayMessage( LocalizableString( "Waiting to start ..." ) );

  // show the user window
  if (m_pForm != NULL)
    m_pForm->Show();
} // Initialize


void UsrEnv::InitializeAlgorithm(TTask * pTask, TApplication * pApplication)
{
  //all the potential elements
  if (m_pUsrElementColl != NULL)
  {
    delete m_pUsrElementColl;
    m_pUsrElementColl = NULL;
  }
  m_pUsrElementColl = new UsrElementCollection();

  // get the active elements as a subset of all the potential elements
  // delete the existing active elements
  if (m_pActiveUsrElementColl != NULL)
  {
    delete m_pActiveUsrElementColl;
    m_pActiveUsrElementColl = NULL;
  }
  m_pActiveUsrElementColl = new UsrElementCollection();
  // create a new collection of elements
  if (m_pAlgorithm != NULL)
    m_pAlgorithm->Initialize(m_pActiveUsrElementColl, m_pUsrElementColl, pTask, pApplication);
} // CreateElementCollection


// **************************************************************************
// Function:   GenerateActiveElements
// Purpose:    Positions the active elements on the screen (elements coordinates 0..65536)
// Parameters: void
// Returns:    void
//
// **************************************************************************
const unsigned int UsrEnv::GenerateActiveElements(const unsigned int & uPhaseInSequence)
{
  unsigned int uReturn(0);
  // get the active elements as a subset of all the potential elements
  // delete the existing active elements
  if (m_pActiveUsrElementColl != NULL)
  {
    delete m_pActiveUsrElementColl;
    m_pActiveUsrElementColl = NULL;
  }
  m_pActiveUsrElementColl = new UsrElementCollection();
  // create a new collection of active elements
  if (m_pAlgorithm != NULL)
    uReturn =
            m_pAlgorithm->GenerateActiveElements(m_pActiveUsrElementColl, m_pUsrElementColl, uPhaseInSequence);
  return uReturn;
} // GenerateActiveElements


// **************************************************************************
// Function:   HideMessage
// Purpose:    To hide the currently displayed message on the screen
// Parameters:  void
// Returns:     void
//
// **************************************************************************
void UsrEnv::HideMessage(void)
{
  if (m_pMessage)
    m_pMessage->Visible = false;
} // HideMessage


// **************************************************************************
// Function:   DisplayMessage
// Purpose:    To display a message on the screen
// Parameters: message - message to be displayed
// Returns:    void
//
// **************************************************************************
void UsrEnv::DisplayMessage(const char * message)
{
  if (m_pForm != NULL)
  {
    const float fScaleX = (float)m_pForm->ClientWidth / 65536.0f;
    const float fScaleY = (float)m_pForm->ClientHeight / 65536.0f;

    const int iScaledTextSize = (int)(8.0f * 655.36f * fScaleY);
    m_pForm->Canvas->Font->Height = -iScaledTextSize;
    m_pForm->Canvas->Font->Name = "Arial";

    // write the congratulations text
    if (m_pMessage != NULL)
    {
      m_pMessage->Font->Color  = clYellow;
      m_pMessage->Font->Height = -iScaledTextSize;
      m_pMessage->Font->Name   = "Arial";
      m_pMessage->Caption      = AnsiString(message);
      m_pMessage->Visible      = true;
      m_pMessage->Layout       = tlBottom;
      m_pMessage->Transparent  = true;
      m_pMessage->Left         = (int)(32767.0f * fScaleX - m_pForm->Canvas->TextWidth(m_pMessage->Caption)/2);
      m_pMessage->Top          = (int)(32767.0f * fScaleY - m_pMessage->Height / 2);
    }
  }
}  // DisplayMessage


// **************************************************************************
// Function:   DisplayElements
// Purpose:    To make active elements visible on the screen
// Parameters: uElementID - what element needs to be displayed
//             if uElementID = 0, display all the element
// Returns:    void
//
// **************************************************************************
void UsrEnv::DisplayElements(const ElementCollEnum eElementColl, const UsrElementCollection::RenderModeEnum eRenderMode,
                                   const unsigned int & uElementID)
{
  if (m_pForm == NULL) return;
  switch (eElementColl)
  {
    case COLL_GENERAL:
      if (m_pUsrElementColl != NULL)
        m_pUsrElementColl->RenderElements(m_pForm, eRenderMode, uElementID,
                                          TRect(0, 0, m_pForm->ClientWidth, m_pForm->ClientHeight));
      break;
    case COLL_ACTIVE:
      if (m_pActiveUsrElementColl != NULL)
        m_pActiveUsrElementColl->RenderElements(m_pForm, eRenderMode, uElementID,
                                          TRect(0, 0, m_pForm->ClientWidth, m_pForm->ClientHeight));
      break;
    case COLL_ALL:
      if (m_pUsrElementColl != NULL)
        m_pUsrElementColl->RenderElements(m_pForm, eRenderMode, uElementID,
                                          TRect(0, 0, m_pForm->ClientWidth, m_pForm->ClientHeight));
      if (m_pActiveUsrElementColl != NULL)
        m_pActiveUsrElementColl->RenderElements(m_pForm, eRenderMode, uElementID,
                                          TRect(0, 0, m_pForm->ClientWidth, m_pForm->ClientHeight));
      break;
    default:
      break;
  }
} // DisplayElements


// **************************************************************************
// Function:   HideElements
// Purpose:    To make active elements invisible on the screen
// Parameters: void
// Returns:    void
//
// **************************************************************************
void UsrEnv::HideElements(const ElementCollEnum eElementColl)
{
  switch (eElementColl)
  {
    case COLL_GENERAL:
      if (m_pUsrElementColl != NULL)
        m_pUsrElementColl->HideElements();
      break;
    case COLL_ACTIVE:
      if (m_pActiveUsrElementColl != NULL)
        m_pActiveUsrElementColl->HideElements();
      break;
    case COLL_ALL:
      if (m_pUsrElementColl != NULL)
        m_pUsrElementColl->HideElements();
      if (m_pActiveUsrElementColl != NULL)
        m_pActiveUsrElementColl->HideElements();
      break;
    default:
      break;
  }
} // HideElements


// **************************************************************************
// Function:   SetWindowSize
// Purpose:    Sets the window size where elements are displayed
// Parameters: iTop - top coordinate
//             iLeft - left coordinate
//             iWidth - width of the form
//             iHeight - height of the form
//             color - color of the form
// Returns:    void
//
// **************************************************************************
void UsrEnv::SetWindowSize(const int & iTop, const int & iLeft,
                           const int & iWidth,const int & iHeight, TColor color)
{
  m_pForm->Top    = iTop;
  m_pForm->Left   = iLeft;
  m_pForm->Width  = iWidth;
  m_pForm->Height = iHeight;
  m_pForm->Color  = color;
} // SetWindowSize


// **************************************************************************
// Function:   GetStateValue
// Purpose:
// Parameters:
// Returns:
//
// **************************************************************************
const int UsrEnv::GetStateValue(const unsigned int & uState)
{
  const int iReturn(0);
  if (m_pAlgorithm)
    return m_pAlgorithm->GetStateValue(uState);
  else
    return iReturn;
} // GetStateValue



