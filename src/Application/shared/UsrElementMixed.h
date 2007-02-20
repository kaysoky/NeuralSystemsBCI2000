/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
#ifndef UsrElementMixedH
#define UsrElementMixedH

#include "UsrElementCaption.h"
#include "UsrElementVideo.h"
#include "UsrElementAudio.h"

class UsrElementMixed : public UsrElementCaption, public UsrElementVideo, public UsrElementAudio
{
public:
  /// Constructors and Destructors
  UsrElementMixed::UsrElementMixed(const unsigned int & uElementID);
  virtual UsrElementMixed::~UsrElementMixed();

  /// Member functions
  virtual UsrElement * GetClone() const;
  virtual void Render(TForm * form, const TRect & destRect);
  virtual void Hide();

};

#endif



