#ifndef UsrEnvDispatcherH
#define UsrEnvDispatcherH
#include <vector>

#define SEQ_ITI                 1
#define SEQ_PTP                 2
#define SEQ_FEEDBACK            3
#define SEQ_OUTCOME             4
#define SEQ_CONGRATULATIONS     5

/// forward declarations
class UsrEnv;
class UsrElement;
class STATEVECTOR;
class PARAMLIST;
class STATELIST;

class UsrEnvDispatcher
{
public:
  enum PhaseInSequenceEnum     // what phase of sequence we are currently running
  {
    PHASE_START = 0,
    PHASE_INTERSTIMULUS,
    PHASE_PRIORSEQUENCE,
    PHASE_STIMULUS,
    PHASE_AFTERSEQUENCE,
    PHASE_FINISH
  };

  enum StateEnum     // what phase of sequence we are currently running
  {
    STATE_SELECTEDSTIMULUS = 0,
    STATE_PHASEINSEQUENCE,
    STATE_STIMULUSTIME,
    STATE_STIMULUSCODE,
    STATE_STIMULUSTYPE,
    STATE_FLASHING
  };
  
  /// Constructors and Destructors
  UsrEnvDispatcher::UsrEnvDispatcher(PARAMLIST * pParamList, STATELIST * pStateList);
  UsrEnvDispatcher::~UsrEnvDispatcher();

  /// Member functions
  void Initialize(PARAMLIST * pParamList, UsrEnv * pUsrEnv, STATEVECTOR * pStateVector);
  void Process(const std::vector<float>&, UsrEnv * pUsrEnv, STATEVECTOR * pStateVector);
  void Reset(UsrEnv * pUsrEnv, STATEVECTOR * pStateVector);
  void SuspendUsrEnv(UsrEnv * pUsrEnv, STATEVECTOR * pStateVector);
  const int ProcessResult(void);

private: 	// User declarations
  unsigned int m_iUsrElementOnTime;      // in units of SampleBlocks
  unsigned int m_iUsrElementOffTime;     // in units of SampleBlocks
  unsigned int m_iUsrElementMinInterTime;      // in units of SampleBlocks
  unsigned int m_iUsrElementMaxInterTime;     // in units of SampleBlocks
  unsigned int m_iUsrElementPriorSeqTime;      // in units of SampleBlocks
  unsigned int m_iUsrElementAfterSeqTime;     // in units of SampleBlocks
  unsigned int m_iCurrentPhaseDuration;  // in units of SampleBlocks (for how long this state is already present)
  unsigned int m_iUsrElementInterStimTime; // in units of SampleBlocks =  m_iUsrElementOffTime + rand(m_iUsrElementMaxInterTime - m_iUsrElementMinInterTime)
  std::vector< bool > m_vStimulusPresent; // whether stimulus is present in the sequence
  bool m_bIsRunning;                // whether we are currently running the experiment
  PhaseInSequenceEnum m_ePhaseInSequence;    // current phase throughout the stimulus sequence
  std::vector< int >   m_vResultCounts; // to accumulate data about how many times a certain stimulus has been selected
  std::vector< float > m_vResultValues; // to accumulate data about what was the actual value
};
#endif
