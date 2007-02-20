/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
#ifndef UsrEnvAlgorithmH
#define UsrEnvAlgorithmH

#include <vcl.h>

/// forward declarations
class UsrElementCollection;
class TTask;

class UsrEnvAlgorithm
{
public:
  /// Constructors and Destructors
  UsrEnvAlgorithm::UsrEnvAlgorithm() {};
  virtual UsrEnvAlgorithm::~UsrEnvAlgorithm() {};

  /// Virtual Member function
  virtual void Initialize(UsrElementCollection * pActiveUsrElementColl,
                          UsrElementCollection * pUsrElementColl, TTask * pTask,
                          TApplication * pApplication) {}
  virtual const unsigned int GenerateActiveElements(UsrElementCollection * pActiveUsrElementColl,
                                                       UsrElementCollection * pUsrElementColl,
                                                       const unsigned int & iPhaseInSequence) { return 0;}
  virtual const int GetStateValue(const unsigned int & uState) { return 0; }
};
#endif




