/////////////////////////////////////////////////////////////////////////////
//
// File: TextProperties.h
//
// Date: Jan 7, 2002
//
// Author: Juergen Mellinger
//
// Description: A mix-in class for views that handle text properties.
//
// Changes:
//
// (C) 2000-2007, BCI2000 Project
// http://www.bci2000.org
//////////////////////////////////////////////////////////////////////////////

#ifndef TEXT_PROPERTIES_H
#define TEXT_PROPERTIES_H

#include <string>

#include "PresErrors.h"
#include "GUITextProperties.h"

class   PARAM;

typedef std::string TInputSequence;
typedef std::string TDisplaySequence;

class TTextProperties : protected TGUITextProperties
{
  protected:
                TTextProperties();
                ~TTextProperties() {}

  public:
    // Use this for setting font properties from a
    // parameter string.
    TPresError  SetTextProperties( const PARAM  *inParamPtr );

    // Use this to convert strings between input and display sequence
    // for right-to-left and bidirectional languages.
    const TInputSequence&   DisplayToInput( const TDisplaySequence& ) const;
    const TDisplaySequence& InputToDisplay( const TInputSequence& ) const;

  private:
    TPresError  ParseTextProperties( const char* inProperties );
};

#endif // TEXT_PROPERTIES_H
