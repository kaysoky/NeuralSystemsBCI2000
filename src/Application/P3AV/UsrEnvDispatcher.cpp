#pragma hdrstop
#pragma package(smart_init)
#include <stdio.h>
#include "PCHIncludes.h"
#include "UParameter.h"
#include "UState.h"
#include "UsrEnvDispatcher.h"
#include "UsrEnv.h"
#include "UBCIError.h"
#include "MeasurementUnits.h"
#include "UsrElementCollection.h"
#include "UsrEnvAlgorithmP3AV.h"
#include "UGenericVisualization.h"
#include "Localization.h"
#include <limits>

// **************************************************************************
// Function:   UsrEnvDispatcher
// Purpose:    This is the constructor of the UsrEnvDispatcher class
// Parameters: plist - pointer to the paramter list
//             slist - pointer to the state list
// Returns:    N/A
// **************************************************************************
UsrEnvDispatcher::UsrEnvDispatcher()
{
  m_iUsrElementOnTime       = 10;
  m_iUsrElementOffTime      = 3;
  m_iUsrElementMinInterTime = 0;
  m_iUsrElementMaxInterTime = 0;
  m_iUsrElementPriorSeqTime = 30;
  m_iUsrElementAfterSeqTime = 30;
  m_iCurrentPhaseDuration   = 0;
  m_iUsrElementInterStimTime= 0;
  m_vStimulusPresent.clear();
  m_bIsRunning              = false;
  m_ePhaseInSequence        = PHASE_START;
  m_vResultCounts.clear();
  m_vResultValues.clear();
  m_iWaitTime = 0;
  m_bWaiting = false;
}


// **************************************************************************
// Function:   ~UsrEnvDispatcher
// Purpose:    This is the destructor of the UsrEnvDispatcher class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
UsrEnvDispatcher::~UsrEnvDispatcher()
{
  m_vStimulusPresent.clear();
  m_vResultCounts.clear();
  m_vResultValues.clear();
}


// **************************************************************************
// Function:   Initialize
// Purpose:    Initialize is called after parameterization to initialize the trial sequence
// Parameters: pParamList        - pointer to the paramter list
//             pNewStateVector    - current pointer to the state vector
//             pNewCorecomm - pointer to the communication object
//             pUsrEnv - pointer to the userdisplay (that contains the status bar, the cursor, and the currently active targets)
// Returns:    0 ... if there was a problem (e.g., a necessary parameter does not exist)
//             1 ... OK
// **************************************************************************
void UsrEnvDispatcher::Initialize(UsrEnv * pUsrEnv)
{
  try
  {
    m_iUsrElementOnTime  = MeasurementUnits::ReadAsTime( Parameter( "OnTime" ) );
    m_iUsrElementOffTime = MeasurementUnits::ReadAsTime( Parameter( "OffTime" ) );
    m_iUsrElementMinInterTime = MeasurementUnits::ReadAsTime( Parameter( "MinInterTime" ) );
    m_iUsrElementMaxInterTime = MeasurementUnits::ReadAsTime( Parameter( "MaxInterTime" ) );
    if (m_iUsrElementMaxInterTime < m_iUsrElementMinInterTime)
    {
      m_iUsrElementMinInterTime = 0;
      m_iUsrElementMaxInterTime = 0;
    }
    m_iUsrElementPriorSeqTime = MeasurementUnits::ReadAsTime( Parameter( "PreSequenceTime" ) );
    m_iUsrElementAfterSeqTime = MeasurementUnits::ReadAsTime( Parameter( "PostSequenceTime" ) );
    if (m_iUsrElementMaxInterTime != m_iUsrElementMinInterTime)
      m_iUsrElementInterStimTime = m_iUsrElementOffTime + m_iUsrElementMinInterTime +
                                    rand() % (m_iUsrElementMaxInterTime - m_iUsrElementMinInterTime);
    else
      m_iUsrElementInterStimTime = m_iUsrElementOffTime + m_iUsrElementMinInterTime;

    // figure out whether a stimulus to appear in a sequence
    m_vStimulusPresent.clear();
    m_vStimulusPresent.resize(pUsrEnv->GetElements()->GetCollectionSize());
    for (unsigned int i(0); i < m_vStimulusPresent.size(); ++i)
      m_vStimulusPresent[i] = false;
    UsrEnvAlgorithmP3AV::SequenceTypeEnum eSequenceType =
          (UsrEnvAlgorithmP3AV::SequenceTypeEnum)int(Parameter("SequenceType"));
    for (unsigned int i = 0; i <  Parameter("Sequence")->GetNumValuesDimension1(); ++i)
    {
      if (eSequenceType == UsrEnvAlgorithmP3AV::SEQUENCE_DETERMINISTIC)
      {
        unsigned int uStimulusID(Parameter("Sequence",i));
        if (uStimulusID <= m_vStimulusPresent.size() && uStimulusID != 0)
          m_vStimulusPresent[uStimulusID - 1] = true;
      }
      else if (eSequenceType == UsrEnvAlgorithmP3AV::SEQUENCE_RANDOM)
      {
        if (i < m_vStimulusPresent.size())
          m_vStimulusPresent[i] = true;
      }
    }

    // find out the wait time
    const unsigned int uWaitTimeSec(1);
    m_iWaitTime = Parameter("SamplingRate") * uWaitTimeSec / Parameter("SampleBlockSize");
    m_bWaiting = false;
  }
  catch(...)
  {
    m_iUsrElementOnTime  = 10;
    m_iUsrElementOffTime = 3;
    m_iUsrElementMinInterTime = 0;
    m_iUsrElementMaxInterTime = 0;
    m_iUsrElementPriorSeqTime = 30;
    m_iUsrElementAfterSeqTime = 30;
    m_iUsrElementInterStimTime = m_iUsrElementOffTime;
    m_vStimulusPresent.clear();
    m_vStimulusPresent.resize(pUsrEnv->GetElements()->GetCollectionSize());
    for (unsigned int i(0); i < m_vStimulusPresent.size(); ++i)
      m_vStimulusPresent[i] = false;
    m_iWaitTime = 0;
    m_bWaiting = false;
  }
  // reset the dispatcher
  Reset(pUsrEnv);
}


// **************************************************************************
// Function:   ResetTrialSequence
// Purpose:    Resets the trial's sequence to the beginning of the ITI
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void UsrEnvDispatcher::Reset(UsrEnv * pUsrEnv)
{
  m_iCurrentPhaseDuration = 0;
  m_ePhaseInSequence      = PHASE_START;
  m_bIsRunning            = false;
  m_bWaiting              = false;

  State( "SelectedStimulus" ) = 0;
  State( "PhaseInSequence" ) = 1;
  State( "StimulusCode" ) = 0;
  State( "StimulusType" ) = 0;
  State( "Flashing" ) = 0;

  if (pUsrEnv != NULL)
  {
    m_vResultCounts.clear();
    m_vResultValues.clear();
    int iStimuliTotalNumber(0);
    if (pUsrEnv->GetElements() != NULL)
      iStimuliTotalNumber = pUsrEnv->GetElements()->GetCollectionSize();
    m_vResultCounts.resize(iStimuliTotalNumber);
    m_vResultValues.resize(iStimuliTotalNumber);
  }
} // Reset


// **************************************************************************
// Function:   SuspendUsrEnv
// Purpose:    Turn off display when trial gets suspended
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void UsrEnvDispatcher::SuspendUsrEnv(UsrEnv * pUsrEnv)
{
  if (pUsrEnv != NULL)
  {
    pUsrEnv->HideElements(UsrEnv::COLL_ALL);   // hide all active elements
    pUsrEnv->DisplayMessage( LocalizableString( "TIME OUT !!!" ) );
  }
  State( "PhaseInSequence" ) = 3;
  State( "StimulusCode" )= 0;
  State( "StimulusType" )= 0;
  State( "Flashing" ) = 0;
  m_vResultCounts.clear();
  m_vResultValues.clear();
}


// **************************************************************************
// Function:   Process
// Purpose:    Processes the control signal provided by the task
// Parameters: controlsignal - pointer to the vector of control signals
// Returns:    pointer to the selected target (if one was selected), or NULL
// **************************************************************************
void UsrEnvDispatcher::Process(const std::vector<float>& controlsignal, UsrEnv * pUsrEnv,
                               GenericVisualization * pGenericVisualization)
{
  if (pUsrEnv == NULL || pGenericVisualization == NULL) return;

  bool bIsStillRunning = State( "Running" );

  // when we suspend the system, show the "TIME OUT" message
  if ((bIsStillRunning == false) && (m_bIsRunning == true))
  {
    SuspendUsrEnv(pUsrEnv);
    m_bIsRunning = false;
  }

  if (bIsStillRunning == false)
    return;

  // when we (re)start the system, reset the trial's sequence
  if ((bIsStillRunning == true) && (m_bIsRunning == false))
  {
    Reset(pUsrEnv);
    pUsrEnv->HideMessage(); // hide any message that's there
    pGenericVisualization->SendMemo2Operator("New run.");
  }
  m_bIsRunning = bIsStillRunning;

  // accumulate statistics
  unsigned short uStimulusCodeRes = State( "StimulusCodeRes" );
  if (uStimulusCodeRes > 0 && uStimulusCodeRes <= (unsigned short)m_vResultCounts.size() &&
      uStimulusCodeRes <= (unsigned short)m_vResultValues.size())
  {
    m_vResultCounts[uStimulusCodeRes - 1]++;
    m_vResultValues[uStimulusCodeRes - 1] += (float)controlsignal[0];
  }

  // whether we need to wait before going to the next phase
  if (m_bWaiting == true)
  {
    if (m_iCurrentPhaseDuration == m_iWaitTime)
    {
      m_iCurrentPhaseDuration = 0;
      m_bWaiting = false;
    }
    else
    {
      ++m_iCurrentPhaseDuration;
      return;
    }
  }
  // right at the beginning of the run
  if (m_ePhaseInSequence == PHASE_START)
  {
    m_ePhaseInSequence = PHASE_PRIORSEQUENCE;
    m_iCurrentPhaseDuration = 0;

    pUsrEnv->HideElements(UsrEnv::COLL_ALL);
    // this will display 'focus on' message
    pUsrEnv->DisplayElements(UsrEnv::COLL_ACTIVE, UsrElementCollection::RENDER_FIRST, 0);

    State( "SelectedStimulus" ) = 0;
    State( "PhaseInSequence" ) = 1;
    State( "StimulusCode" ) = 0;
    State( "StimulusType" ) = 0;
    State( "Flashing" ) = 0;

    pGenericVisualization->SendMemo2Operator("New sequence is presented.");
  }

  // prior to sequence
  if (m_ePhaseInSequence == PHASE_PRIORSEQUENCE)
  {
    if (m_iCurrentPhaseDuration == m_iUsrElementPriorSeqTime)
    {
      m_ePhaseInSequence = PHASE_INTERSTIMULUS;
      m_iCurrentPhaseDuration = 0;
      m_bWaiting = true;
      if (m_iUsrElementMaxInterTime != m_iUsrElementMinInterTime)
        m_iUsrElementInterStimTime = m_iUsrElementOffTime + m_iUsrElementMinInterTime +
                                      rand() % (m_iUsrElementMaxInterTime - m_iUsrElementMinInterTime);
      else
        m_iUsrElementInterStimTime = m_iUsrElementOffTime + m_iUsrElementMinInterTime;

      pUsrEnv->HideElements(UsrEnv::COLL_ACTIVE);
      // get the next active elements as a subset of all the potential elements
      m_ePhaseInSequence = (PhaseInSequenceEnum)(pUsrEnv->GenerateActiveElements((unsigned int)m_ePhaseInSequence));

      State( "SelectedStimulus" ) = 0;
      State( "PhaseInSequence" ) = 0;
      State( "StimulusCode" ) = 0;
      State( "StimulusType" ) = 0;
      State( "Flashing" ) = 0;
    }
    else if (m_iCurrentPhaseDuration == (m_iUsrElementPriorSeqTime - m_iUsrElementOnTime))
    {
      pUsrEnv->HideElements(UsrEnv::COLL_ACTIVE);
      // this will display what element we have to focus on message
      pUsrEnv->DisplayElements(UsrEnv::COLL_ACTIVE, UsrElementCollection::RENDER_LAST, 0);
      if (pUsrEnv->GetActiveElements() != NULL)
        if (pUsrEnv->GetActiveElements()->GetElementPtrByIndex(1) != NULL)
        {
          char memotext[256];
          sprintf(memotext, "Focusing on stimulus %d \r", pUsrEnv->GetActiveElements()->GetElementID(1));
          pGenericVisualization->SendMemo2Operator(memotext);
        }
    }
  }


  // element is currently present and its time expired
  if (m_ePhaseInSequence == PHASE_STIMULUS && (m_iCurrentPhaseDuration == m_iUsrElementOnTime))
  {
    m_ePhaseInSequence = PHASE_INTERSTIMULUS;
    m_iCurrentPhaseDuration = 0;
    if (m_iUsrElementMaxInterTime != m_iUsrElementMinInterTime)
      m_iUsrElementInterStimTime = m_iUsrElementOffTime + m_iUsrElementMinInterTime +
                                      rand() % (m_iUsrElementMaxInterTime - m_iUsrElementMinInterTime);
    else
      m_iUsrElementInterStimTime = m_iUsrElementOffTime + m_iUsrElementMinInterTime;

    pUsrEnv->HideElements(UsrEnv::COLL_ACTIVE);
    // get the next active elements as a subset of all the potential elements
    m_ePhaseInSequence = (PhaseInSequenceEnum)pUsrEnv->GenerateActiveElements((unsigned int)m_ePhaseInSequence);

    State("SelectedStimulus")=0;
    State("PhaseInSequence")=0;
    State("StimulusCode")=0;
    State("StimulusType")=0;
    State("Flashing")=0;

    // wait before displaying the "result was" message
    if (m_ePhaseInSequence == PHASE_AFTERSEQUENCE)
    {
      m_bWaiting = true;
      State("PhaseInSequence")=3;
    }
  }

  // element is not present and interstimulus time expired
  if (m_ePhaseInSequence == PHASE_INTERSTIMULUS && (m_iCurrentPhaseDuration == m_iUsrElementInterStimTime))
  {
    m_ePhaseInSequence = PHASE_STIMULUS;
    m_iCurrentPhaseDuration = 0;

    pUsrEnv->DisplayElements(UsrEnv::COLL_ACTIVE, UsrElementCollection::RENDER_ALL, 0);

    State("SelectedStimulus")=0;
    State("PhaseInSequence")=2;
    State("Flashing")=1;
    if (pUsrEnv->GetActiveElements() != NULL)
    {
      State("StimulusCode")=pUsrEnv->GetActiveElements()->GetElementID(0);
      // figure out stimulus type here
      State("StimulusType")=pUsrEnv->GetStateValue(STATE_STIMULUSTYPE);
    }
  }

  // after sequence phase
  if (m_ePhaseInSequence == PHASE_AFTERSEQUENCE && m_bWaiting == false)
  {
    if (m_iCurrentPhaseDuration == 0)
    {
      pUsrEnv->HideElements(UsrEnv::COLL_ACTIVE);
      PhaseInSequenceEnum ePhaseInSequence = (PhaseInSequenceEnum)pUsrEnv->GenerateActiveElements((unsigned int)m_ePhaseInSequence);
      // this will display 'result is' message
      if (ePhaseInSequence == PHASE_FINISH)
         pUsrEnv->DisplayElements(UsrEnv::COLL_ACTIVE, UsrElementCollection::RENDER_FIRST, 0);

      if ((ePhaseInSequence == PHASE_FINISH) || (ePhaseInSequence == PHASE_FINISH_WO_RESULT))
        m_ePhaseInSequence = ePhaseInSequence;

      State("SelectedStimulus")=0;
      State("PhaseInSequence")=3;
      State("StimulusCode")=0;
      State("StimulusType")=0;
      State("Flashing")=0;
    }
    else if (m_iCurrentPhaseDuration == m_iUsrElementAfterSeqTime)
    {
      m_iCurrentPhaseDuration = 0;
      m_ePhaseInSequence = PHASE_START;
      m_bWaiting = true;

      pUsrEnv->HideElements(UsrEnv::COLL_ALL);
      pUsrEnv->GenerateActiveElements((unsigned int)PHASE_PRIORSEQUENCE);

      State("SelectedStimulus")=0;
      State("PhaseInSequence")=1;
      State("StimulusCode")=0;
      State("StimulusType")=0;
      State("Flashing")=0;
    }
    else if (m_iCurrentPhaseDuration == (m_iUsrElementAfterSeqTime - m_iUsrElementOnTime))
    {
      pUsrEnv->HideElements(UsrEnv::COLL_ACTIVE);
      const int iPickedStimulusID = ProcessResult(pGenericVisualization);  // display result
      pUsrEnv->DisplayElements(UsrEnv::COLL_GENERAL, UsrElementCollection::RENDER_SPECIFIC_ID, iPickedStimulusID);

      State("SelectedStimulus")=iPickedStimulusID;
    } // else
  }  // if (m_ePhaseInSequence == PHASE_AFTERSEQUENCE)

  // the end
  if ((m_ePhaseInSequence == PHASE_FINISH) || (m_ePhaseInSequence == PHASE_FINISH_WO_RESULT))
  {
    if (m_iCurrentPhaseDuration == (m_iUsrElementAfterSeqTime - m_iUsrElementOnTime))
    {
      pUsrEnv->HideElements(UsrEnv::COLL_ACTIVE);
      if (m_ePhaseInSequence == PHASE_FINISH)
         {
         const int iPickedStimulusID = ProcessResult(pGenericVisualization);  // display result
         pUsrEnv->DisplayElements(UsrEnv::COLL_GENERAL, UsrElementCollection::RENDER_SPECIFIC_ID, iPickedStimulusID);
         State("SelectedStimulus")=iPickedStimulusID;
         }
      if (m_ePhaseInSequence == PHASE_FINISH_WO_RESULT)        // 1/28/04 GS
         State("SelectedStimulus")=0;
      State("PhaseInSequence")=3;
      State("StimulusCode")=0;
      State("StimulusType")=0;
      State("Flashing")=0;
    }
    else if (m_iCurrentPhaseDuration == m_iUsrElementAfterSeqTime)
    {
      m_iCurrentPhaseDuration = 0;
      State("Running")=0;
    }
  }

  // time of the current state
  m_iCurrentPhaseDuration++;
}


const int UsrEnvDispatcher::ProcessResult(GenericVisualization * pGenericVisualization)
{
  for (unsigned int i = 0; i < m_vStimulusPresent.size(); ++i)
  {
    // report error
    if (m_vStimulusPresent[i] == true && m_vResultCounts[i] == 0)
      bcierr << "Signal processing module didnt send a result for stimulus number " << AnsiString(i + 1).c_str() << std::endl;
  }
  // process the results
  float fMaxValue = - std::numeric_limits<float>::max();
  int iPickedStimulusID(0);
  for (unsigned int i = 0; i < m_vResultCounts.size(); ++i)
  {
    if (m_vResultCounts[i] > 0)
      if (m_vResultValues[i]/(float)m_vResultCounts[i] > fMaxValue)
      {
        fMaxValue = m_vResultValues[i]/(float)m_vResultCounts[i];
        iPickedStimulusID = i + 1;
      }
  }
  // clear statictics
  for (unsigned int i(0); i < m_vResultCounts.size(); ++i)
  {
    m_vResultCounts[i] = 0;
    m_vResultValues[i] = 0;
  }
  if (pGenericVisualization != NULL && iPickedStimulusID != 0)
  {
    char memotext[256];
    // send the results to the operator log
    sprintf(memotext, "The predicted stimulus was stimulus %d \r", iPickedStimulusID);
    pGenericVisualization->SendMemo2Operator(memotext);
  }
  return iPickedStimulusID;
}

