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
//          - Created file.
//          - Moved UpdateState() from ../shared/UState.h to here.
//          June 10, 2004, juergen.mellinger@uni-tuebingen.de:
//          - Removed UpdateState() -- use TfMain::UpdateState() instead.
//          Dec 30, 2004, juergen.mellinger@uni-tuebingen.de:
//          - Moved User Level accessors and Matrix I/O functions from TfMain
//            to here.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef UOperatorUtilsH
#define UOperatorUtilsH

class Controls::TControl;
class PARAM;

#undef ERR_NOERR
enum {
  ERR_NOERR = 0,
  ERR_MATLOADCOLSDIFF,
  ERR_MATNOTFOUND,
  ERR_COULDNOTWRITE,
};

namespace OperatorUtils
{
// **************************************************************************
// Function:   SaveControl
// Purpose:    Write control properties to the registry.
// Parameters: A pointer to a TControl object.
// Returns:    n/a
// **************************************************************************
  void SaveControl( const TControl* );

// **************************************************************************
// Function:   RestoreControl
// Purpose:    Restore control properties from the registry.
// Parameters: A pointer to a TControl object.
// Returns:    n/a
// **************************************************************************
  void RestoreControl( TControl* );

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
  int LoadMatrix( const char* inFileName, PARAM& outParam );

// **************************************************************************
// Function:   SaveMatrix
// Purpose:    Saves a matrix to a file, delimited by white spaces
// Parameters: - filename of the matrix file, containing the full path
//             - pointer to the parameter that contains the matrix
// Returns:    ERR_NOERR - no error
//             ERR_COULDNOTWRITE - could not write matrix to output file
// **************************************************************************
  int SaveMatrix( const char* inFileName, const PARAM& inParam );

};

#endif // UOperatorUtilsH
