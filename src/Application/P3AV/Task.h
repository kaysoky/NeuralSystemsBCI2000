#ifndef TaskH
#define TaskH

#include "UGenericFilter.h"
#include "UsrEnv.h";

/// forward declarations
class GenericVisualization;
class UsrEnv;
class BCITIME;
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
  const bool ErrorReadingMatrix(const AnsiString asMatrixName) const;
  const bool ErrorLoadingAudioFile(std::string sAudioFile) const;
  const bool ErrorLoadingVideoFile(std::string sVideoFile) const;

private:
  /// Member variables
  GenericVisualization * m_pGenericVisualization;
  UsrEnv * m_pUsrEnv;
  UsrEnvDispatcher * m_pUsrEnvDispatcher;
  BCITIME * m_pBCITime;
};

#endif
