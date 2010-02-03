/////////////////////////////////////////////////////////////////////////////
//
// File: PresErrors.h
//
// Date: Oct 15, 2001
//
// Author: Juergen Mellinger
//
// Description:
//
// Changes:
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
//////////////////////////////////////////////////////////////////////////////

#ifndef PRESERRORSH
#define PRESERRORSH

#include <sstream>

typedef enum TPresError
{
    firstPresError = 0,
    presNoError = 0,
    presGenError,
    presParamInaccessibleError,
    presParamOutOfRangeError,
    presFileOpeningError,
    presIllegalSpellerTreeError,
    presResNotFoundError,

    presNumErrors
} TPresError;

class TGUITextDialog;

class TErrorReport : public std::ostringstream
{
  public:
            TErrorReport();
            TErrorReport(   const char  *inWindowTitle );
            ~TErrorReport();

    void    AddError(   TPresError  inError,
                        const char  *inTextArgument );
    void    AddError(   TPresError  inError,
                        const char  *inTextArgument,
                        float       inNumArgument );

    void    DisplayErrors();

  private:
    std::string     windowTitle;
    TGUITextDialog  *errorDialog;
};

extern  TErrorReport    gPresErrors;

#endif // PRESERRORSH
