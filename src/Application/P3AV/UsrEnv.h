#ifndef UserEnvH
#define UserEnvH

#include "UsrElementCollection.h"

/// forward declarations
class UsrEnvAlgorithm;
class PARAMLIST;
class TTask;

class UsrEnv
{
public:

  enum ElementCollEnum
  {
    COLL_GENERAL = 0,
    COLL_ACTIVE,
    COLL_ALL
  };
  /// Constructors and Destructors
  UsrEnv::UsrEnv(const AnsiString & asTaskName, UsrEnvAlgorithm * pAlgorithm);
  UsrEnv::~UsrEnv();

  /// Sets and Gets
  // Set and get for active elements collection
  inline UsrElementCollection * GetActiveElements(void) { return m_pActiveUsrElementColl; }
  inline UsrElementCollection * GetElements(void) { return m_pUsrElementColl; }

  /// Member functions
  void Initialize(TTask * pTask, TApplication * pApplication, const int & iTop, const int & iLeft,
                  const int & iWidth,const int & iHeight, TColor color);
  void HideElements(const ElementCollEnum eElementColl);
  void DisplayElements(const ElementCollEnum eElementColl, const UsrElementCollection::RenderModeEnum eRenderMode,
                       const unsigned int & uElementID);
  void DisplayMessage(const char * message);
  void HideMessage(void);
  void InitializeAlgorithm(TTask * pTask, TApplication * pApplication);
  const unsigned int GenerateActiveElements(const unsigned int & uPhaseInSequence);
  void SetWindowSize(const int & iTop, const int & iLeft,
                     const int & iWidth,const int & iHeight, TColor color);
  const int GetStateValue(const unsigned int & uState);

private:
  /// member variables
  UsrElementCollection * m_pUsrElementColl;
  UsrElementCollection * m_pActiveUsrElementColl;
  UsrEnvAlgorithm * m_pAlgorithm;
  TLabel * m_pMessage;
  TForm * m_pForm;
};
#endif
