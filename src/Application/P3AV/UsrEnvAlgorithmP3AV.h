#ifndef UsrEnvAlgorithmP3AVH
#define UsrEnvAlgorithmP3AVH

#include "UsrEnvAlgorithm.h"
#include "UEnvironment.h"

/// forward declarations
class UsrElementCollection;
class UsrElement;
class UsrElementMixed;

class UsrEnvAlgorithmP3AV : public UsrEnvAlgorithm, private Environment
{
public:
  enum SequenceTypeEnum
  {
    SEQUENCE_DETERMINISTIC = 0,
    SEQUENCE_RANDOM = 1
  };

  enum InterpretationModeEnum
  {
    INTERPRETATION_NONE = 0,
    INTERPRETATION_FREEMODE,
    INTERPRETATION_COPYMODE
  };
  
public:
  /// Constructors and Destructors
  UsrEnvAlgorithmP3AV::UsrEnvAlgorithmP3AV();
  virtual UsrEnvAlgorithmP3AV::~UsrEnvAlgorithmP3AV();

  /// Virtual Member function
  virtual void Initialize(UsrElementCollection * pActiveUsrElementColl,
                          UsrElementCollection * pUsrElementColl,
                          class TTask*, class TApplication* );
  virtual const unsigned int GenerateActiveElements(UsrElementCollection * pActiveUsrElementColl,
                                                      UsrElementCollection * pUsrElementColl,
                                                      const unsigned int & iPhaseInSequence);
  virtual const int GetStateValue(const unsigned int & uState);
private:
  /// Private Member functions
  void Reset(void);
  void CreateElementCollection(UsrElementCollection * pUsrElementColl,
                               const float & fWidthInPercentOfScreen);
  void CreateStimuliSequence();
  void CreateMiscellaneousElements(const float & fWidthInPercentOfScreen);
  void CreateStimuliSequenceToBeCopied();
  void AddToActiveElements(UsrElementCollection * pActiveUsrElementColl, UsrElement * pUsrElement);
  
private:
  std::list< int > m_listStimuliSequence;
  std::list< int > m_listStimuliToBeCopiedSeq;
  unsigned int m_uCurStimuliSequenceIndex;
  unsigned int m_uCurStimuliToBeCopiedSeqIndex;
  unsigned int m_uCurStimuliToBeCopiedID;
  bool m_bStimulusTypeValue;
  unsigned int m_uNumOfTimesToPlay;
  unsigned int m_uNumOfTimesToCopy;
  unsigned int m_uNumOfTimesCopied;
  UsrElementMixed * m_pFocusOnElement;
  UsrElementMixed * m_pResultElement;
  InterpretationModeEnum m_eInterpretMode;
};
#endif


