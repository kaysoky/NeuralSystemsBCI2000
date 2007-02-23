/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
#ifndef UsrElementCollectionH
#define UsrElementCollectionH

#include <syncobjs.hpp>
#include "UsrElementMixed.h"
#include <list>

class UsrElementCollection
{
public:
   // is used to tell the DisplayElements function what element/s needs to be displayed
  enum SelectionModeEnum
  {
    SELECT_FIRST = 0,
    SELECT_SPECIFIC_ID,
    SELECT_ALL,
    SELECT_LAST
  };
  
  /// Constructors and Destructors
  UsrElementCollection::UsrElementCollection();
  UsrElementCollection::~UsrElementCollection();

  /// Sets and Gets
  inline const unsigned int GetCollectionSize(void) { return m_usrElementList.size(); }

  /// Member function
  const bool AddElement(UsrElement * pElement);
  void DeleteElements(void);
  void HideElements(void);
  void ShowElements( SelectionModeEnum inSelectionMode, unsigned int inElementID );
  void RenderElements( TCanvas& inCanvas, const TRect& inDestRect) const;
  UsrElement * GetElementPtrByID(const unsigned int & uElementID) const;
  UsrElement * GetElementPtrByIndex(const unsigned int & uIndex) const;
  const unsigned int GetElementID(const unsigned int & uIndex) const;
  UsrElement * SelectedElement(CURSOR *cursor) const;
  const unsigned int GetNumElements(void) const;

private:
  /// Member variables
  TCriticalSection * m_pCritSec; // critical section for screen update
  std::list<UsrElement *> m_usrElementList; // list of elements
};

#endif



