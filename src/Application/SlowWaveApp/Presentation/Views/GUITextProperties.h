/////////////////////////////////////////////////////////////////////////////
//
// File: GUITextProperties.h
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

#ifndef GUI_TEXT_PROPERTIES_H
#define GUI_TEXT_PROPERTIES_H

#include "GUI.h"
#include "OSIncludes.h"

#ifndef VCL
#include <string>
#endif // VCL

class TGUITextProperties
{
  protected:
    // Only derived classes may instantiate and use this class.
            TGUITextProperties()  {}
            ~TGUITextProperties() {}

    void    SetFontName(   const char       *inName );
    void    SetFontSize(   float            inSize );
    void    SetFontStyle(  TGUIFontStyle    inStyle );
    void    SetFontColor(  TGUIColor        inColor );
    void    SetAlignment(  TGUIAlignment    inAlignment );
    void    SetVAlignment( TGUIVAlignment   inVAlignment );
    void    SetBiDi(       TGUIBiDi         inBiDi );

    float           fontSize;
    TGUIVAlignment  vTextAlignment;
#ifdef VCL
    AnsiString      fontName;
    TFontStyles     fontStyle;
    TColor          fontColor;
    TAlignment      textAlignment;
#else // VCL
    std::string     fontName;
    TGUIFontStyle   fontStyle;
    TGUIColor       fontColor;
    TGUIAlignment   textAlignment;
#endif // VCL
    TGUIBiDi        biDi;
};

#endif // GUI_TEXT_PROPERTIES_H
