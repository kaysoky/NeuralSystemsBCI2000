#ifndef UsrElementCaptionH
#define UsrElementCaptionH

#include "UsrElement.h"

class UsrElementCaption : public virtual UsrElement
{
public:
  /// Constructors and Destructors
  UsrElementCaption::UsrElementCaption(const unsigned int & uElementID);
  virtual UsrElementCaption::~UsrElementCaption();
  
  /// Sets and Gets
  void SetCaptionBkgdColor(TColor bkgdColor);
  inline TColor GetCaptionBkgdColor(void) const { return m_bkgdColor; }

  void SetCaptionText(const AnsiString & asText);
  inline const AnsiString & GetCaptionText(void) const { return m_asText; }

  void SetCaptionTextColor(TColor textColor);
  inline TColor GetCaptionTextColor(void) const { return m_textColor; }

  void SetCaptionTextHeight(const unsigned int & uTextHeight);
  inline const unsigned int & GetCaptionTextHeight(void) const { return m_uTextHeight; }

  void SetCaptionAttributes(TColor bkgdColor, const AnsiString & asText, TColor textColor, const unsigned int & uTextHeight);
  
  /// Member functions
  virtual UsrElement * GetClone() const;
  virtual void Render(TForm * form, const TRect & destRect);
  virtual void Hide();

private:
  /// Member variables
  TLabel  * m_pLabel;
  AnsiString m_asText;
  TColor m_bkgdColor;
  TColor m_textColor;
  unsigned int m_uTextHeight;
};

#endif

