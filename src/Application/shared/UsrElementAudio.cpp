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
  cloned=false;                 // cloned UsrElementAudio only "borrowed" the WavePlayer and don't delete it in the destructor
} // UsrElementAudio


// **************************************************************************
// Function:   ~UsrElementAudio
// Purpose:    This is the destructor of the UsrElementAudio class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
UsrElementAudio::~UsrElementAudio()
{
  // if an element was cloned using CloneAudio(), it uses the WavePlayer from the source element
  // thus, this assumes that the source element is not deleted before the cloned element
  if ((m_pAudio != NULL) && (!cloned))
  {
    delete m_pAudio;
    m_pAudio = NULL;
  }
} // ~UsrElementAudio


// **************************************************************************
// Function:   GetClone
// Purpose:    This function clones a particular element by "deep-copying" the old one
//             This version of cloning loads the audio file in the clone process (as opposed to CloneAudio())
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
  // plays the audio
  if (m_pAudio != NULL)
  {
    Hide();
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
// Function:   SetAudioFileName
// Purpose:    This function sets the audio file name
// Parameters: asAudioFile - file name
// Returns:    N/A
// **************************************************************************
void UsrElementAudio::SetAudioFileName(const AnsiString & asAudioFile)
{
  m_asAudioFile = asAudioFile;

  // create the audio object, if not already exists
  if ((m_asAudioFile != "") && (m_pAudio == NULL))
  {
    m_pAudio = new TWavePlayer();
    if (m_pAudio) m_pAudio->AttachFile(m_asAudioFile.c_str());
  }
} // SetAudioFileName


// **************************************************************************
// Function:   GetAudioFileName
// Purpose:    This function returns the audio file name
// Parameters: N/A
// Returns:    audio file name
// **************************************************************************
const AnsiString & UsrElementAudio::GetAudioFileName(void) const
{
  return m_asAudioFile;
} // GetAudioFileName


// **************************************************************************
// Function:   GetAudioWavePlayer
// Purpose:    This function returns a pointer to the wave player
// Parameters: N/A
// Returns:    ptr to wave player
// **************************************************************************
const TWavePlayer * UsrElementAudio::GetAudioWavePlayer(void) const
{
  return m_pAudio;
} // GetAudioWavePlayer


// **************************************************************************
// Function:   CloneAudio
// Purpose:    This function clones a specified UsrElementAudio element
//             The cloned element contains a pointer to the original
//             TWavePlayer; when an element is cloned, this TWavePlayer is
//             NOT deleted. However, this assumes that the "original"
//             UsrElementAudio element will not be deleted before the cloned element
// Parameters: UsrElementAudio *src = pointer to the source element
// Returns:    N/A
// **************************************************************************
void UsrElementAudio::CloneAudio(const UsrElementAudio *src)
{
 SetCoordsRect(src->GetCoordsRect());

 m_asAudioFile=src->GetAudioFileName();
 m_pAudio=(TWavePlayer *)src->GetAudioWavePlayer();
 cloned=true;
} // CloneAudio



