#pragma hdrstop
#pragma package(smart_init)
#include "UsrElementAudio.h"
#include "WavePlayer.h"

// **************************************************************************
// Function:   UsrElementAudio
// Purpose:    This is the constructor of the UsrElementAudio class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
UsrElementAudio::UsrElementAudio(const unsigned int & uElementID) : UsrElement(uElementID)
{
  m_pAudio = NULL;
  m_asAudioFile = "";
} // UsrElementAudio


// **************************************************************************
// Function:   ~UsrElementAudio
// Purpose:    This is the destructor of the UsrElementAudio class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
UsrElementAudio::~UsrElementAudio()
{
  if (m_pAudio != NULL)
  {
    delete m_pAudio;
    m_pAudio = NULL;
  }
} // ~UsrElementAudio


// **************************************************************************
// Function:   GetClone
// Purpose:    This function clones a particular element by "deep-copying" the old one
// Parameters: N/A
// Returns:    pointer to the cloned element
// **************************************************************************
UsrElement * UsrElementAudio::GetClone(void) const
{
  UsrElementAudio * pClonedElement = new UsrElementAudio(this->GetID());
  pClonedElement->SetAudioFileName(this->GetAudioFileName());
  pClonedElement->SetCoordsRect(this->GetCoordsRect());

  return pClonedElement;
}


// **************************************************************************
// Function:   Render
// Purpose:    This function plays music for the element
// Parameters: form     - pointer to the form that will hold the element
//             destRect - part of the form the element will be rendered into
// Returns:    N/A
// **************************************************************************
void UsrElementAudio::Render(TForm * form, const TRect & destRect)
{
  // create the audio object, if not already exists
  if ((m_asAudioFile != "") && (m_pAudio == NULL))
  {
    m_pAudio = new TWavePlayer();
  }

  // plays the audio
  if (m_pAudio != NULL)
  {
    Hide();
    m_pAudio->AttachFile(m_asAudioFile.c_str());
    m_pAudio->Play();
  }
}   // Render


// **************************************************************************
// Function:   Hide
// Purpose:    This function hides this element
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void UsrElementAudio::Hide(void)
{
  // hide the icon, if it exists
  if (m_pAudio != NULL)
    if (m_pAudio->IsPlaying())
      m_pAudio->Stop();
}


// **************************************************************************
// Function:   SetIconFileName
// Purpose:    This function sets the audio file name
// Parameters: asAudioFile - file name
// Returns:    N/A
// **************************************************************************
void UsrElementAudio::SetAudioFileName(const AnsiString & asAudioFile)
{
  m_asAudioFile = asAudioFile;
} // SetIconFileName


// **************************************************************************
// Function:   GetAudioFileName
// Purpose:    This function returns the audio file name
// Parameters: N/A
// Returns:    audio file name
// **************************************************************************
const AnsiString & UsrElementAudio::GetAudioFileName(void) const
{
  return m_asAudioFile;
} // GetIconFileName


