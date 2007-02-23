/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
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
  bool Visible() const { return m_bVisible; }

  /// Virtual Member function
  virtual UsrElement * GetClone() const = 0;
  virtual void Render( TCanvas& ioCanvas, const TRect& inDestRect ) const = 0;
  virtual void Show() { m_bVisible = true; }
  virtual void Hide() { m_bVisible = false; }

private:
  /// Member variables
  unsigned int m_uElementID; // element ID
  TRect        m_rectElementCoords; // coords of element expressed by TRect class
  bool         m_bVisible; // determines whether element will be rendered or not.
};

#endif



