#pragma hdrstop
#pragma package(smart_init)
#include "PCHIncludes.h"
#include <stdio.h>
#include "UsrEnvAlgorithmP3AV.h"
#include "UsrElementCollection.h"
#include "Task.h"
#include "UBCIError.h"
#include "UTaskUtil.h"
#include "UsrEnvDispatcher.h"

// **************************************************************************
// Function:   UsrEnvAlgorithmP3AV
// Purpose:    Constructor
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
UsrEnvAlgorithmP3AV::UsrEnvAlgorithmP3AV() : UsrEnvAlgorithm()
{
  m_pFocusOnElement = NULL;
  m_pResultElement = NULL;
  m_listStimuliSequence.clear();
  m_listStimuliToBeCopiedSeq.clear();
  Reset();
} // UsrEnvAlgorithmP3AV


// **************************************************************************
// Function:   ~UsrEnvAlgorithmP3AV
// Purpose:    Destructor
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
UsrEnvAlgorithmP3AV::~UsrEnvAlgorithmP3AV()
{
  Reset();
} // ~UsrEnvAlgorithmP3AV


// **************************************************************************
// Function:   Reset
// Purpose:    Resets the member variables
// Parameters: void
// Returns:
// **************************************************************************
void UsrEnvAlgorithmP3AV::Reset(void)
{
  // delete list of stimuli
  if (m_listStimuliSequence.size() != 0)
  {
    m_listStimuliSequence.erase(m_listStimuliSequence.begin(), m_listStimuliSequence.end());
    m_listStimuliSequence.clear();
  }

  // delete list of stimuli  to be copied
  if (m_listStimuliToBeCopiedSeq.size() != 0)
  {
    m_listStimuliToBeCopiedSeq.erase(m_listStimuliToBeCopiedSeq.begin(), m_listStimuliToBeCopiedSeq.end());
    m_listStimuliToBeCopiedSeq.clear();
  }

  // delete focus on element
  if (m_pFocusOnElement != NULL)
    delete m_pFocusOnElement;
  m_pFocusOnElement = NULL;
  // delete result element
  if (m_pResultElement != NULL)
    delete m_pResultElement;
  m_pResultElement = NULL;

  m_uCurStimuliSequenceIndex = 0;
  m_uCurStimuliToBeCopiedSeqIndex = 0;
  m_uNumOfTimesToPlay = 0;
  m_uNumOfTimesToCopy = 0;
  m_uNumOfTimesCopied = 0;
  m_eInterpretMode = INTERPRETATION_NONE;
  m_uCurStimuliToBeCopiedID = 0;
  m_bStimulusTypeValue = false;
} // Reset


// **************************************************************************
// Function:   Initialize
// Purpose:    Create a list of usr element available in the environment
//             Creates a sequence of stimuli that is going to be played in the usr env
//             Creates a bunch of other miscellaneous elements specific for the task
// Parameters: pUsrElementCollection - the list from which elements need to be extracted
//             pTask  -  a task that is currently being executed
//             pApplication - application that is currently running
// Returns:
// **************************************************************************
void UsrEnvAlgorithmP3AV::Initialize(UsrElementCollection * pActiveUsrElementColl,
                                     UsrElementCollection * pUsrElementColl, TTask * pTask,
                                     TApplication * pApplication)
{
  if (pTask != NULL && pApplication != NULL && pUsrElementColl != NULL)
  {
    // find out about some of the user settings for the elements
    float fWidthInPercentOfScreen = pTask->Parameter( "StimulusWidth" );

    // reset all member variables
    Reset();

    // set some of the member varaibles to new values according to
    // parameters values
    // number of times the sequence has to be played
    m_uNumOfTimesToPlay = pTask->Parameter("NumberOfSeq");
    m_eInterpretMode = (InterpretationModeEnum)((int)pTask->Parameter( "InterpretMode" ));

    // creates a collection of elements existing inside the usr env
    CreateElementCollection(pUsrElementColl, pTask, pApplication, fWidthInPercentOfScreen);

    // create a stimuli sequence
    CreateStimuliSequence(pTask);

    // create focuson and result elements
    CreateMiscellaneousElements(pTask, pApplication, fWidthInPercentOfScreen);

    // create a list of stimuli to be copied in copy mode
    CreateStimuliSequenceToBeCopied(pTask);

    // finally create active elements
    GenerateActiveElements(pActiveUsrElementColl, pUsrElementColl, UsrEnvDispatcher::PHASE_PRIORSEQUENCE);
  }
} // CreateElementCollection


// **************************************************************************
// Function:   CreateElementCollection
// Purpose:    Create a list of usr element available in the environment
// Parameters: pUsrElementCollection - the list from which elements need to be extracted
//             pTask  -  a task that is currently being executed
//             pApplication - application that is currently running
// Returns:
// **************************************************************************
void UsrEnvAlgorithmP3AV::CreateElementCollection(UsrElementCollection * pUsrElementColl, TTask * pTask,
                                                  TApplication * pApplication, const float & fWidthInPercentOfScreen)
{
  if (pTask != NULL && pApplication != NULL && pUsrElementColl != NULL)
  {
    // create a collection
    AnsiString applicationPath = ExtractFilePath(pApplication->ExeName);
    for( size_t i = 0; i <  pTask->Parameter( "Matrix" )->GetNumValuesDimension2(); ++i )
    {
      AnsiString iconFileName(AnsiString(( const char* )pTask->Parameter( "Matrix", "icon", i )));
      if (iconFileName != "")
        iconFileName = applicationPath + iconFileName;

      AnsiString soundFileName(AnsiString(( const char* )pTask->Parameter( "Matrix", "audio", i )));
      if (soundFileName != "")
        soundFileName = applicationPath + soundFileName;

      UsrElementMixed * pUsrElementMixed = new UsrElementMixed(i + 1);
       // initialize the position of elements
      pUsrElementMixed->SetCoordsRect(fWidthInPercentOfScreen, 1.0f, false);
      // check caption switch
      if (pTask->Parameter( "CaptionSwitch" ) == 1)
        pUsrElementMixed->SetCaptionText(AnsiString(( const char* )pTask->Parameter( "Matrix", "caption", i )));
      else
        pUsrElementMixed->SetCaptionText(AnsiString(""));
      pUsrElementMixed->SetCaptionTextHeight(pTask->Parameter( "CaptionHeight" ));
      // check audio switch
      if (pTask->Parameter( "AudioSwitch" ) == 1)
        pUsrElementMixed->SetAudioFileName(soundFileName);
      else
        pUsrElementMixed->SetAudioFileName("");
      // check video switch
      if (pTask->Parameter( "VideoSwitch" ) == 1)
        pUsrElementMixed->SetIconFileName(iconFileName);
      else
        pUsrElementMixed->SetIconFileName("");

      pUsrElementColl->AddElement(pUsrElementMixed);
    }
  }
}// CreateElementCollection


// **************************************************************************
// Function:   CreateStimuliSequnce
// Purpose:    Creates a stimuli sequence according to which elements are shown on the screen
// Parameters: pTask -  a task that is currently being executed
// Returns:
// **************************************************************************
void UsrEnvAlgorithmP3AV::CreateStimuliSequence(TTask * pTask)
{
  if (pTask != NULL)
  {
    if (m_uNumOfTimesToPlay == 0) return;
    SequenceTypeEnum eSequenceType =
                    (pTask->Parameter("SequenceType") == 0) ? SEQUENCE_DETERMINISTIC : SEQUENCE_RANDOM;

    if (eSequenceType == SEQUENCE_DETERMINISTIC)
    {
      for (unsigned int uNumber(0); uNumber < m_uNumOfTimesToPlay; ++uNumber)
      {
        for (size_t i = 0; i <  pTask->Parameter("Sequence")->GetNumValuesDimension1(); ++i)
          m_listStimuliSequence.push_back(pTask->Parameter("Sequence", i));
      }
    }
    else if (eSequenceType == SEQUENCE_RANDOM)
    {
      // create temporary Stimuli sequnce of type 1, 1, 1, 1, 2, 2, 3, 3, 3 where
      // first stimulus is repeated as many times as specified in the parameter list
      std::list<int> listStimuliSequnceTemp;
      listStimuliSequnceTemp.clear();
      for (size_t i = 0; i <  pTask->Parameter("Sequence")->GetNumValuesDimension1(); ++i)
      {
        unsigned int uNumOfTimesToRepeatStimulus(pTask->Parameter("Sequence", i));
        for (unsigned int k(0); k < uNumOfTimesToRepeatStimulus; ++k)
          listStimuliSequnceTemp.push_back(i + 1);
      }

      InitializeBlockRandomizedNumber();
      // now get a permutation of the temporary stimuli sequence
      unsigned int uBlockSize(listStimuliSequnceTemp.size());
      for (unsigned int uNumber(0); uNumber < m_uNumOfTimesToPlay; ++uNumber)
      {
        unsigned int uNumOfPermutations(1);

        // perform uBlockSize number of permutations
        while (uNumOfPermutations <= uBlockSize)
        {
          unsigned int uWhatElementNeedsToBeCopied(GetBlockRandomizedNumber(uBlockSize) - 1);
          // Search the list for the element to be erased
          std::list< int >::iterator iter;
          unsigned int i(0);
          for (iter = listStimuliSequnceTemp.begin(); iter != listStimuliSequnceTemp.end(); ++iter, ++i)
          {
            if (uWhatElementNeedsToBeCopied == i)
            {
              int uValueOfElementWhichNeedsToBeCopied(*iter);
              m_listStimuliSequence.push_back(uValueOfElementWhichNeedsToBeCopied);
              break;
            }
          }
          ++uNumOfPermutations;
        } // while (uNumOfPermutations <= uBlockSize)
      } // for (unsigned int uNumOfTimesToPlay(0)
      listStimuliSequnceTemp.clear();
    } // else if (eSequenceType == SEQUENCE_RANDOM)
  }
} // CreateStimuliSequnce


// **************************************************************************
// Function:   CreateMiscellaneousElements
// Purpose:    Used to create FocusOn and Result elements. FocusOn is announced in
//             the beginning of sequence, Result is announced in the end of a sequence
// Parameters: pTask - the list from which elements need to be extracted
//             pApplication - application that is currently running
// Returns:
// **************************************************************************
void UsrEnvAlgorithmP3AV::CreateMiscellaneousElements(TTask * pTask, TApplication * pApplication,
                                                      const float & fWidthInPercentOfScreen)
{
  if (pTask != NULL && pApplication != NULL)
  {
    if (m_eInterpretMode != INTERPRETATION_NONE)
    {
      // focus on element
      AnsiString applicationPath = ExtractFilePath(pApplication->ExeName);
      AnsiString iconFileName = AnsiString(( const char* )pTask->Parameter( "FocusOn", "icon", 0 ));
      if (iconFileName != "")
        iconFileName = applicationPath + iconFileName;
      AnsiString soundFileName = AnsiString(( const char* )pTask->Parameter( "FocusOn", "audio", 0 ));
      if (soundFileName != "")
        soundFileName = applicationPath + soundFileName;
      if (m_pFocusOnElement != NULL)
        delete m_pFocusOnElement;
      m_pFocusOnElement = NULL;
      m_pFocusOnElement = new UsrElementMixed(1);
      m_pFocusOnElement->SetCoordsRect(fWidthInPercentOfScreen, 1.0f, false);
      m_pFocusOnElement->SetCaptionText(AnsiString(( const char* )pTask->Parameter( "FocusOn", "caption", 0 )));
      m_pFocusOnElement->SetCaptionTextHeight(pTask->Parameter( "CaptionHeight" ));
      m_pFocusOnElement->SetAudioFileName(soundFileName);
      m_pFocusOnElement->SetIconFileName(iconFileName);

      // result element
      iconFileName = AnsiString(( const char* )pTask->Parameter( "Result", "icon", 0 ));
      if (iconFileName != "")
        iconFileName = applicationPath + iconFileName;
      soundFileName = AnsiString(( const char* )pTask->Parameter( "Result", "audio", 0 ));
      if (soundFileName != "")
        soundFileName = applicationPath + soundFileName;
      if (m_pResultElement != NULL)
        delete m_pResultElement;
      m_pResultElement = NULL;
      m_pResultElement = new UsrElementMixed(1);
      m_pResultElement->SetCoordsRect(fWidthInPercentOfScreen, 1.0f, false);
      m_pResultElement->SetCaptionText(AnsiString(( const char* )pTask->Parameter( "Result", "caption", 0 )));
      m_pResultElement->SetCaptionTextHeight(pTask->Parameter( "CaptionHeight" ));
      m_pResultElement->SetAudioFileName(soundFileName);
      m_pResultElement->SetIconFileName(iconFileName);
    }
  }
}// CreateMiscellaneousElements


// **************************************************************************
// Function:   CreateStimuliSequenceToBeCopied
// Purpose:    If we are in copy mode, we need to know the sequence according to which
//             the stimuli will be copied
// Parameters: pTask - the list from which elements need to be extracted
// Returns:    void
// **************************************************************************
void UsrEnvAlgorithmP3AV::CreateStimuliSequenceToBeCopied(TTask * pTask)
{
  if (pTask != NULL)
    if (m_eInterpretMode == INTERPRETATION_COPYMODE)
    {
       for (size_t i = 0; i <  pTask->Parameter("ToBeCopied")->GetNumValuesDimension1(); ++i)
          m_listStimuliToBeCopiedSeq.push_back(pTask->Parameter("ToBeCopied", i));
       m_uNumOfTimesToCopy = m_listStimuliToBeCopiedSeq.size();
    }
}// CreateStimuliSequenceToBeCopied


// **************************************************************************
// Function:   GenerateActiveElements
// Purpose:    Extract from the list elements which are currently active
//             based on certain algorithm for generation of active elements
// Parameters: pUsrElementCollection - the list from which elements need to be extracted
//             pActiveUsrElementColl  -  a list of active elements
// Returns:
// **************************************************************************
const unsigned int UsrEnvAlgorithmP3AV::GenerateActiveElements(UsrElementCollection * pActiveUsrElementColl,
                                                                  UsrElementCollection * pUsrElementColl,
                                                                  const unsigned int & iPhaseInSequence)
{
  unsigned int iNewPhaseInSequence(iPhaseInSequence);
  m_bStimulusTypeValue = false;
  if (pUsrElementColl != NULL && pActiveUsrElementColl != NULL &&
      ((m_uNumOfTimesToPlay != 0 && m_eInterpretMode != INTERPRETATION_COPYMODE) ||
       (m_uNumOfTimesToPlay != 0 && m_eInterpretMode == INTERPRETATION_COPYMODE && m_uNumOfTimesToCopy != 0)))
  {
    UsrEnvDispatcher::PhaseInSequenceEnum ePhaseInSequence = (UsrEnvDispatcher::PhaseInSequenceEnum)iPhaseInSequence;
    switch (ePhaseInSequence)
    {
      case UsrEnvDispatcher::PHASE_PRIORSEQUENCE:
        if (m_eInterpretMode == INTERPRETATION_COPYMODE)
        {
          // add to active targets element: please focus on....
          AddToActiveElements(pActiveUsrElementColl, dynamic_cast<UsrElement *>(m_pFocusOnElement));
          // now add element which is in the stimuli to be copied list
          UsrElement * pUsrElement = NULL;
          std::list<int>::iterator iter;
          unsigned int i(0);
          for (iter = m_listStimuliToBeCopiedSeq.begin(); iter != m_listStimuliToBeCopiedSeq.end(); ++iter, ++i)
            if (m_uCurStimuliToBeCopiedSeqIndex == i)
            {
              pUsrElement = pUsrElementColl->GetElementPtrByID(*iter);
              m_uCurStimuliToBeCopiedID = *iter;
              ++m_uCurStimuliToBeCopiedSeqIndex;
              break;
            }
          // if we found such an element, add it to the active elements list
          AddToActiveElements(pActiveUsrElementColl, pUsrElement);
        } // if (m_eInterpretMode == INTERPRETATION_COPYMODE)
        iNewPhaseInSequence = (unsigned int)UsrEnvDispatcher::PHASE_INTERSTIMULUS;
        break;


      case UsrEnvDispatcher::PHASE_AFTERSEQUENCE:
        // we are dealing with INTERPRETATION_COPYMODE
        // in that case we want to proceed with the next sequence but before we need to
        // announce the results
        if (m_eInterpretMode != INTERPRETATION_NONE) // add to active targets element: the result is...
          AddToActiveElements(pActiveUsrElementColl, dynamic_cast<UsrElement *>(m_pResultElement));
        if (m_eInterpretMode == INTERPRETATION_COPYMODE)
          ++m_uNumOfTimesCopied;
        if (m_uNumOfTimesCopied == m_uNumOfTimesToCopy && m_eInterpretMode == INTERPRETATION_COPYMODE)
           iNewPhaseInSequence = (unsigned int)UsrEnvDispatcher::PHASE_FINISH;
        else
           iNewPhaseInSequence = (unsigned int)UsrEnvDispatcher::PHASE_PRIORSEQUENCE;
        if (m_eInterpretMode == INTERPRETATION_NONE)        // 01/28/04 GS
           iNewPhaseInSequence = (unsigned int)UsrEnvDispatcher::PHASE_FINISH_WO_RESULT;
        break;

      case UsrEnvDispatcher::PHASE_INTERSTIMULUS:
      case UsrEnvDispatcher::PHASE_STIMULUS:
      {
        // if we are at the end of sequence, reset the current stimuli sequence index to zero
        // otherwise increment by one
        if (m_uCurStimuliSequenceIndex == m_listStimuliSequence.size())
        {
          m_uCurStimuliSequenceIndex = 0;
          // include the following line if we want stimulus to never stop in no interpretation mode
          // if (m_eInterpretMode != INTERPRETATION_NONE)       // 01/28/04 GS
             return iNewPhaseInSequence = (unsigned int)UsrEnvDispatcher::PHASE_AFTERSEQUENCE;
        }
        // Search the list for the element which corresponds to the current index in the
        // stimuli sequence
        UsrElement * pUsrElement = NULL;
        std::list<int>::iterator iter;
        unsigned int i(0);
        for (iter = m_listStimuliSequence.begin(); iter != m_listStimuliSequence.end(); ++iter, ++i)
          if (m_uCurStimuliSequenceIndex == i)
          {
            pUsrElement = pUsrElementColl->GetElementPtrByID(*iter);
            break;
          }
        // if we found such an element, add it to the active elements list
        if (pUsrElement != NULL)
        {
          AddToActiveElements(pActiveUsrElementColl, pUsrElement);
          ++m_uCurStimuliSequenceIndex;
          iNewPhaseInSequence = iPhaseInSequence;
          m_bStimulusTypeValue = (m_uCurStimuliToBeCopiedID == pUsrElement->GetID());
        } // if (pUsrElement != NULL)
        break;
      }
      default:
        break;
    }
  }
  return iNewPhaseInSequence;
} // GenerateActiveElements


void UsrEnvAlgorithmP3AV::AddToActiveElements(UsrElementCollection * pActiveUsrElementColl, UsrElement * pUsrElement)
{
  if (pUsrElement != NULL && pActiveUsrElementColl != NULL)
  {
    // add to active targets
    UsrElement * pClonedUsrElement = pUsrElement->GetClone();
    if (pClonedUsrElement != NULL)
    {
      if (UsrElementCaption * pUsrElementCaption = dynamic_cast<UsrElementCaption *>(pClonedUsrElement))
      {
        pUsrElementCaption->SetCaptionBkgdColor(clYellow);
        pUsrElementCaption->SetCaptionTextColor(clBlack);
      }
      pActiveUsrElementColl->AddElement(pClonedUsrElement);
    }// if (pClonedUsrElement != NULL)
  }//if (pUsrElement != NULL)
} // AddToActiveElements


const int UsrEnvAlgorithmP3AV::GetStateValue(const unsigned int & uState)
{
  UsrEnvDispatcher::StateEnum eState = (UsrEnvDispatcher::StateEnum)uState;
  int iReturn(0);
  switch (eState)
  {
    case UsrEnvDispatcher::STATE_STIMULUSTYPE:
      if (m_eInterpretMode == INTERPRETATION_COPYMODE && m_uCurStimuliToBeCopiedSeqIndex > 0)
        iReturn = m_bStimulusTypeValue;
      break;
    default:
      break;
  }
  return iReturn;
} // GetStateValue

