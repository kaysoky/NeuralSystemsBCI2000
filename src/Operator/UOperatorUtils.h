////////////////////////////////////////////////////////////////////////////////
//
// File: UOperatorUtils.cpp
//
// Date: June 27, 2002
//
// Description: A file intended to hold global utility functions common to
//              different operator sources.
//
// Changes: June 27, 2002, juergen.mellinger@uni-tuebingen.de:
//          Created file.
//          Moved UpdateState() from ../shared/UState.h to here.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef UOperatorUtilsH
#define UOperatorUtilsH

class STATELIST;

namespace OperatorUtils
{
// **************************************************************************
// Function:   SaveControl
// Purpose:    Write control properties to the registry.
// Parameters: A pointer to a TControl object.
// Returns:    n/a
// **************************************************************************
  void   SaveControl( const TControl* );

// **************************************************************************
// Function:   RestoreControl
// Purpose:    Restore control properties from the registry.
// Parameters: A pointer to a TControl object.
// Returns:    n/a
// **************************************************************************
  void   RestoreControl( TControl* );
};

#endif
