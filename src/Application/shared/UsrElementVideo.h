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
  const TImage * GetIconImage(void) const;

  /// Member functions
  virtual UsrElement * GetClone() const;
  virtual void Render(TForm * form, const TRect & destRect);
  virtual void Hide();
  
private:

  TImage * m_pIcon;
  AnsiString m_asIconFile;
};

#endif

