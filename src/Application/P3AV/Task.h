#ifndef TaskH
#define TaskH

#include "UGenericFilter.h"
#include "UGenericVisualization.h"
#include "UsrEnv.h"

/// forward declarations
class UsrEnv;
class UsrEnvDispatcher;

class TTask : public GenericFilter
{
 friend class UsrEnvAlgorithmP3AV;
public:
  /// Constructors and Destructors
  TTask();
  virtual ~TTask();

  /// Virtual Member functions
  virtual void Preflight(const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize(void);
  virtual void Process(const GenericSignal * pInput, GenericSignal * pOutput );

  /// Member functions
  bool ErrorReadingMatrix( const std::string& sMatrixName ) const;
  bool ErrorLoadingAudioFile( const std::string& sAudioFile ) const;
  bool ErrorLoadingVideoFile( const std::string& sVideoFile) const;

private:
  /// Member variables
  std::string mApplicationPath;
  GenericVisualization mTaskLogVis;
  UsrEnv * m_pUsrEnv;
  UsrEnvDispatcher * m_pUsrEnvDispatcher;
  char  GetPressedKey();
};

#endif
