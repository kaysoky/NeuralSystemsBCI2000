#ifndef UsrElementH
#define UsrElementH

#include <vcl.h>

/// forward declarations
class CURSOR;

class UsrElement
{
public:
  /// Constructors and Destructors
  UsrElement::UsrElement(const unsigned int & uElementID);
  virtual UsrElement::~UsrElement();

  /// Sets and Gets
  // Set and Get for Element coords rectangle
  inline void SetCoordsRect(const TRect & rectElementCoords) { m_rectElementCoords = rectElementCoords; }
  void SetCoordsRect(const float & fWidthInPercentOfScreen, const float & fAspectRatio, const bool bUseExistingWidth);
  inline const TRect & GetCoordsRect(void) const { return m_rectElementCoords; }
  // Set and Get for element ID
  inline void SetID(const unsigned int & uElementID) { m_uElementID = uElementID; }
  inline const unsigned int & GetID(void) const { return m_uElementID; }
  
  /// Member functions
  const TRect GetScaledCoordsRect(const TRect & destRect) const ;
  const bool SelectionSucceeded(CURSOR * pCursor) const;

  /// Virtual Member function
  virtual UsrElement * GetClone() const;
  virtual void Render(TForm * form, const TRect & destRect) {};
  virtual void Hide() {};

private:
  /// Member variables
  unsigned int m_uElementID; // element ID
  TRect        m_rectElementCoords; // coords of element expressed by TRect class

};

#endif

