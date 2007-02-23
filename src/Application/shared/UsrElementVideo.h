/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
#ifndef UsrElementVideoH
#define UsrElementVideoH

#include "UsrElement.h"

class UsrElementVideo : public virtual UsrElement
{
public:
  /// Constructors and Destructors
  UsrElementVideo::UsrElementVideo(const unsigned int & uElementID);
  virtual UsrElementVideo::~UsrElementVideo();
  
  /// Sets and Gets
  // Get and Set for Icon file name
  void SetIconFileName(const AnsiString & asIconFile);
  void CloneIcon(const UsrElementVideo *src);
  const AnsiString & GetIconFileName(void) const;
  const TPicture * GetIconPicture(void) const;

  /// Member functions
  virtual UsrElement * GetClone() const;
  virtual void Render( TCanvas& ioCanvas, const TRect& inDestRect ) const;

private:
  TPicture * m_pIcon;
  AnsiString m_asIconFile;
};

#endif



