#ifndef UsrElementAudioH
#define UsrElementAudioH

#include "UsrElement.h"

/// forward declaration
class TWavePlayer;

class UsrElementAudio : public virtual UsrElement
{
public:
  /// Constructors and Destructors
  UsrElementAudio::UsrElementAudio(const unsigned int & uElementID);
  virtual UsrElementAudio::~UsrElementAudio();
  
  /// Sets and Gets
  // Get and Set for audio file name
  void SetAudioFileName(const AnsiString & asAudioFile);
  const AnsiString & GetAudioFileName(void) const;
  
  /// Member functions
  virtual UsrElement * GetClone() const;
  virtual void Render(TForm * form, const TRect & destRect);
  virtual void Hide();
  
private:

  TWavePlayer * m_pAudio;
  AnsiString m_asAudioFile;
};

#endif

