/////////////////////////////////////////////////////////////////////////////
//
// File: PresErrors.cpp
//
// Date: Nov 22, 2001
//
// Author: Juergen Mellinger
//
// Description:
//
// Changes:
//
//////////////////////////////////////////////////////////////////////////////

#ifdef __BORLANDC__
#include "PCHIncludes.h"
#pragma hdrstop
#endif // __BORLANDC__

#include "PresErrors.h"
#include "Utils/GUITextDialog.h"
#include "ParamDecl.h"

TErrorReport    gPresErrors( "Presentation errors" );

using namespace std;

TErrorReport::TErrorReport()
: errorDialog( NULL )
{
}

TErrorReport::TErrorReport( const char  *inWindowTitle )
: windowTitle( inWindowTitle ),
  errorDialog( NULL )
{
}

TErrorReport::~TErrorReport()
{
    delete errorDialog;
}

void
TErrorReport::AddError( TPresError  inError,
                        const char  *inTextArgument )
{
    switch( inError )
    {
        case presNoError:
            break;

        case presParamInaccessibleError:
            *this << "Parameter '"
                  << inTextArgument << "' inaccessible." << endl;
            break;

        case presParamOutOfRangeError:
            *this << "Parameter '"
                  << inTextArgument << "' out of range." << endl;
            break;

        case presFileOpeningError:
            *this << "Error when trying to open the file '"
                  << inTextArgument << "'." << endl;
            break;

        case presIllegalSpellerTreeError:
            *this << "File '"
                  << inTextArgument
                  << "' contains an illegal speller tree definition." << endl;
            break;

        case presResNotFoundError:
            *this << "Unable to open resource '"
                  << inTextArgument << "'." << endl;
            break;
            
        case presGenError:
        default:
            *this << "Unspecific Error related to '"
                  << inTextArgument << "'." << endl;
    }
}

void
TErrorReport::AddError( TPresError  inError,
                        const char  *inTextArgument,
                        float       inNumArgument )
{
    switch( inError )
    {
        case presNoError:
            break;

        case presParamInaccessibleError:
        case presParamOutOfRangeError:
            {
                string          paramName( inTextArgument );
                unsigned int    replacePos = paramName.find( RUNTIME_SUFFIX );
                if( replacePos != string::npos )
                {
                    ostringstream   suffix;
                    suffix << inNumArgument;
                    paramName.replace( replacePos, sizeof( RUNTIME_SUFFIX ) - 1, suffix.str() );
                }
                *this << "Parameter '" << paramName << "' ";
                switch( inError )
                {
                    case presParamInaccessibleError:
                        *this << "inaccessible";
                        break;
                    case presParamOutOfRangeError:
                        *this << "out of range";
                        break;
                }
                *this << "." << endl;
            }
            break;

        case presFileOpeningError:
            *this << "Error when trying to open the file '"
                  << inTextArgument << "'."
                  << " (Error code is " << inNumArgument << ")" << endl;
            break;

        case presIllegalSpellerTreeError:
            *this << "File '"
                  << inTextArgument
                  << "' contains an illegal speller tree definition "
                  << "(Error in line " << inNumArgument << ")." << endl;
            break;

        case presResNotFoundError:
            *this << "Unable to open resource '" << inTextArgument
                  << inNumArgument << "'." << endl;
            break;

        case presGenError:
        default:
            *this << "Unspecific Error related to '"
                  << inTextArgument << "' and '"
                  << inNumArgument << "'." << endl;
    }
}

void
TErrorReport::DisplayErrors()
{
    if( !str().empty() )
    {
        if( errorDialog == NULL )
            errorDialog = new TGUITextDialog;
        errorDialog->ShowTextNonmodal( windowTitle.c_str(), str().c_str() );
    }
    str( "" );
}

