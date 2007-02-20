/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
#pragma hdrstop
#pragma package(smart_init)
#include "PCHIncludes.h"
#include "UBCIError.h"
#include "UCursor.h"
#include "UsrElementCollection.h"

// **************************************************************************
// Function:   UsrElementCollection
// Purpose:    This is the constructor for the UsrElementCollection class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
UsrElementCollection::UsrElementCollection()
{
  m_usrElementList.clear();
  m_pCritSec = new TCriticalSection;
} // UsrElementCollection



// **************************************************************************
// Function:   ~UsrElementCollection
// Purpose:    This is the destructor for the UsrElementCollection class
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
UsrElementCollection::~UsrElementCollection()
{
  DeleteElements();
  if (m_pCritSec != NULL)
  {
    delete m_pCritSec;
    m_pCritSec = NULL;
  }
} // ~UsrElementCollection


// **************************************************************************
// Function:   AddElement
// Purpose:    This function adds a new element to the list
// Parameters: pElement - new element
// Returns:    true  - on success
//             false - on failure (e.g., element already exists)
// **************************************************************************
const bool UsrElementCollection::AddElement(UsrElement * pElement)
{
 // add element to the list of elements
 m_pCritSec->Acquire();
 m_usrElementList.push_back(pElement);
 m_pCritSec->Release();
 return true;
}  // AddElement


// **************************************************************************
// Function:   DeleteTargets
// Purpose:    This function clears the list of elements and frees the associated memory
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void UsrElementCollection::DeleteElements(void)
{
  if (m_pCritSec == NULL || m_usrElementList.empty())
    return;

  m_pCritSec->Acquire();

  // Clean up – must free memory for the items as well as the list
  std::list<UsrElement *>::const_iterator elementIter;
  for (elementIter = m_usrElementList.begin(); elementIter != m_usrElementList.end(); ++elementIter)
    delete (*elementIter);  

  m_usrElementList.clear();

  m_pCritSec->Release();
} // DeleteElements


// **************************************************************************
// Function:   HideElements
// Purpose:    This function hides all elements
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void UsrElementCollection::HideElements(void)
{
  m_pCritSec->Acquire();

  std::list<UsrElement *>::const_iterator elementIter;
  for (elementIter = m_usrElementList.begin(); elementIter != m_usrElementList.end(); ++elementIter)
  {
    if ((*elementIter) != NULL)
      (*elementIter)->Hide();
  }

  m_pCritSec->Release();
}  // HideElements


// **************************************************************************
// Function:   RenderElements
// Purpose:    This function renders all elements onto the specified canvas
// Parameters: canvas - pointer to the canvas that will hold the elements
// Returns:    N/A
// **************************************************************************
void UsrElementCollection::RenderElements(TForm * form, const RenderModeEnum eRenderMode,
                                          const unsigned int & uElementID, const TRect & destRect)
{
  m_pCritSec->Acquire();

  std::list<UsrElement *>::const_iterator elementIter;
  switch (eRenderMode)
  {
    case RENDER_FIRST:
      if (m_usrElementList.size() != 0)
      {
        elementIter = m_usrElementList.begin();
        if ((*elementIter) != NULL)
          (*elementIter)->Render(form, destRect);
      }
      break;
    case RENDER_SPECIFIC_ID:
      for (elementIter = m_usrElementList.begin(); elementIter != m_usrElementList.end(); ++elementIter)
        if ((*elementIter) != NULL)
          if ((*elementIter)->GetID() == uElementID)
          {
            (*elementIter)->Render(form, destRect);
            break;
          }
      break;
    case RENDER_ALL:
      for (elementIter = m_usrElementList.begin(); elementIter != m_usrElementList.end(); ++elementIter)
        if ((*elementIter) != NULL)
          (*elementIter)->Render(form, destRect);
      break;
    case RENDER_LAST:
      {
        std::list<UsrElement *>::reverse_iterator relementIter;
        if (m_usrElementList.size() != 0)
        {
          relementIter = m_usrElementList.rbegin();
          if ((*relementIter) != NULL)
            (*relementIter)->Render(form, destRect);
        }
      }
      break;
    default:
      break;
  }
  
  m_pCritSec->Release();
} // RenderElements


// **************************************************************************
// Function:   SelectedElement
// Purpose:    This function detects whether there is collision with ANY element
// Parameters: cursor - pointer to the cursor
// Returns:    pointer to the element, if it has been selected
//             NULL, if no element has been selected
// **************************************************************************
UsrElement * UsrElementCollection::SelectedElement(CURSOR *cursor) const
{
  m_pCritSec->Acquire();

  UsrElement * pSelectedElement = NULL;
  std::list<UsrElement *>::const_iterator elementIter;
  for (elementIter = m_usrElementList.begin(); elementIter != m_usrElementList.end(); ++elementIter)
  {
    if ((*elementIter) != NULL)
    {
      if ((*elementIter)->SelectionSucceeded(cursor) == true)
      {
        pSelectedElement = *elementIter;
        break;
      }
    }
  }

  m_pCritSec->Release();
  return pSelectedElement;
}  // SelectedElement


// **************************************************************************
// Function:   GetElementPtr
// Purpose:    This function returns the pointer of a element specified by its ID
// Parameters: uElementID - ID of sought after element
// Returns:    pointer of element - on success
//             NULL              - on failure (i.e., element not found)
// **************************************************************************
UsrElement * UsrElementCollection::GetElementPtrByID(const unsigned int & uElementID) const
{
  m_pCritSec->Acquire();

  UsrElement * pElement = NULL;
  std::list<UsrElement *>::const_iterator elementIter;
  for (elementIter = m_usrElementList.begin(); elementIter != m_usrElementList.end(); ++elementIter)
  {
    if ((*elementIter) != NULL)
    {
      if ((*elementIter)->GetID() == uElementID)
      {
        pElement = *elementIter;
        break;
      }
    }
  }

  m_pCritSec->Release();
  return pElement;
} // GetElementPtrByID


// **************************************************************************
// Function:   GetElementPtr
// Purpose:    This function returns the pointer of a element specified by its index
// Parameters: uIndex - index of sought after element
// Returns:    pointer of element - on success
//             NULL              - on failure (i.e., element not found)
// **************************************************************************
UsrElement * UsrElementCollection::GetElementPtrByIndex(const unsigned int & uIndex) const
{
  m_pCritSec->Acquire();

  UsrElement * pElement = NULL;
  unsigned int i(0);
  std::list<UsrElement *>::const_iterator elementIter;
  for (elementIter = m_usrElementList.begin(); elementIter != m_usrElementList.end(); ++elementIter, ++i)
  {
    if ((*elementIter) != NULL && i == uIndex)
    {
        pElement = *elementIter;
        break;
    }
  }

  m_pCritSec->Release();
  return pElement;
} // GetElementPtrByIndex


// **************************************************************************
// Function:   GetElementID
// Purpose:    This function returns the ID of the element specified by its display position
// Parameters: uIndex - index of sought after element
// Returns:    elementID ... success
//             9999 - on failure (i.e., element not found)
// **************************************************************************
const unsigned int UsrElementCollection::GetElementID(const unsigned int & uIndex) const
{
  m_pCritSec->Acquire();
  unsigned int uElementID(9999);
  std::list<UsrElement *>::const_iterator elementIter;
  unsigned int i(0);
  for (elementIter = m_usrElementList.begin(); elementIter != m_usrElementList.end(); ++elementIter, ++i)
  {
    if ((*elementIter) != NULL && i == uIndex)
    {
      uElementID = (*elementIter)->GetID();
      break;
    }
  }
  m_pCritSec->Release();
  return uElementID;
} // GetElementID


// **************************************************************************
// Function:   GetNumElements
// Purpose:    This function returns the number of elements in this list
// Parameters: N/A
// Returns:    number of elements in the list
// **************************************************************************
const unsigned int UsrElementCollection::GetNumElements(void) const
{
  return m_usrElementList.size();
} // GetNumElements


