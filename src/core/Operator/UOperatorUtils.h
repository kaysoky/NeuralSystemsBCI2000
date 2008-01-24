////////////////////////////////////////////////////////////////////////////////
// $Id$
// Description: A file intended to hold global utility functions common to
//              various operator source files.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////

#ifndef UOperatorUtilsH
#define UOperatorUtilsH

#include <string>

class Controls::TControl;
class Param;

namespace OperatorUtils
{
// **************************************************************************
// Function:   SaveControl
// Purpose:    Write control properties to the registry.
// Parameters: A pointer to a TControl object.
// Returns:    n/a
// **************************************************************************
  void SaveControl( const TControl*, const std::string& AppKey );

// **************************************************************************
// Function:   RestoreControl
// Purpose:    Restore control properties from the registry.
// Parameters: A pointer to a TControl object.
// Returns:    n/a
// **************************************************************************
  void RestoreControl( TControl*, const std::string& AppKey );

// **************************************************************************
// Function:   UserLevel
// Purpose:    Get the global user level.
// Parameters: n/a
// Returns:    Global user level.
// **************************************************************************
  int UserLevel();

// **************************************************************************
// Function:   GetUserLevel
// Purpose:    Read a parameter's user level from the registry.
// Parameters: A pointer to the parameter's name.
// Returns:    User level.
// **************************************************************************
  int GetUserLevel( const char* Name );

// **************************************************************************
// Function:   SetUserLevel
// Purpose:    Change a parameter's user level in the registry.
// Parameters: A pointer to the parameter's name, and the user level.
// Returns:    n/a
// **************************************************************************
  void SetUserLevel( const char* Name, int UserLevel );

// **************************************************************************
// Function:   LoadMatrix
// Purpose:    Loads a matrix that is delimited by white spaces
// Parameters: - filename of the matrix file, containing the full path
//             - pointer to the parameter that contains the matrix
// Returns:    ERR_NOERR - no error
//             ERR_MATLOADCOLSDIFF - number of columns in different rows is different
//             ERR_MATNOTFOUND - could not open input matrix file or file contains no data
// **************************************************************************
  int LoadMatrix( const char* inFileName, Param& outParam );

// **************************************************************************
// Function:   SaveMatrix
// Purpose:    Saves a matrix to a file, delimited by white spaces
// Parameters: - filename of the matrix file, containing the full path
//             - pointer to the parameter that contains the matrix
// Returns:    ERR_NOERR - no error
//             ERR_COULDNOTWRITE - could not write matrix to output file
// **************************************************************************
  int SaveMatrix( const char* inFileName, const Param& inParam );

};

#endif // UOperatorUtilsH
