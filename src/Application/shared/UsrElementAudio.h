/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
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
  
  /// sets and gets
  // Get and Set for audio file name
  void SetAudioFileName(const AnsiString & asAudioFile);
  const AnsiString  & GetAudioFileName(void) const;
  const TWavePlayer * GetAudioWavePlayer(void) const;
  void  CloneAudio(const UsrElementAudio *src);

  /// Member functions
  virtual UsrElement * GetClone() const;
  virtual void Render(TForm * form, const TRect & destRect);
  virtual void Hide();
  
private:

  TWavePlayer * m_pAudio;
  AnsiString m_asAudioFile;
  bool  cloned;
};
#endif



