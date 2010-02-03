/////////////////////////////////////////////////////////////////////////////
//
// File: GUI.h
//
// Date: Oct 22, 2001
//
// Author: Juergen Mellinger
//
// Description: This file contains generalized GUI types and constants like
//              colors, font styles etc.
//
// Changes:
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
//////////////////////////////////////////////////////////////////////////////

#ifndef GUI_H
#define GUI_H

#include "PresErrors.h"
class Param;

typedef enum TGUIColor
{
  firstGUIColor = 0,
  white = 0,
  black = 1,
  red,
  green,
  blue,
  yellow,
  magenta,
  cyan,
  darkblue,
  numGUIColors,
  defaultGUIBGColor = white,
  defaultGUIFGColor = black
} TGUIColor;

#ifdef GUI_DEFINE_OBJECTS
const char* GUIColorStrings[] =
{
  "white",
  "black",
  "red",
  "green",
  "blue",
  "yellow",
  "magenta",
  "cyan",
  "darkblue"
}; // color names as strings
#else
extern const char* GUIColorStrings[];
#endif // GUI_DEFINE_OBJECTS

#ifdef GUI_DEFINE_OBJECTS
const char* GUIDefaultFont = "Arial";
#else
extern const char* GUIDefaultFont;
#endif // GUI_DEFINE_OBJECTS

typedef enum TGUIFontStyle
{
  firstGUIFontStyle = 0,
  plain = 0,
  bold,
  italic,
  bolditalic,
  numGUIFontStyles,
  defaultGUIFontStyle = plain
} TGUIFontStyle;

#ifdef GUI_DEFINE_OBJECTS
const char* GUIFontStyleStrings[] =
{
  "plain",
  "bold",
  "italic",
  "bolditalic"
}; // font style names as strings
#else
extern const char* GUIFontStyleStrings[];
#endif // GUI_DEFINE_OBJECTS

const float GUIDefaultFontSize = 0.1;

typedef enum TGUIAlignment
{
  firstGUIAlignment = 0,
  left = 0,
  right,
  center,
  numGUIAlignments,
  defaultGUIAlignment = 0 //left
} TGUIAlignment;

#ifdef GUI_DEFINE_OBJECTS
const char* GUIAlignmentStrings[] =
{
  "left",
  "right",
  "center"
}; // alignment names as strings
#else
extern const char*  GUIAlignmentStrings[];
#endif // GUI_DEFINE_OBJECTS

typedef enum TGUIVAlignment
{
  firstGUIVAlignment = 0,
  top = 0,
  bottom,
  vcenter,
  numGUIVAlignments,
  defaultGUIVAlignment = top
} TGUIVAlignment;

#ifdef GUI_DEFINE_OBJECTS
const char* GUIVAlignmentStrings[] =
{
  "top",
  "bottom",
  "vcenter"
}; // valignment names as strings
#else
extern const char*  GUIVAlignmentStrings[];
#endif // GUI_DEFINE_OBJECTS

typedef enum TGUIBiDi
{
  firstGUIBiDi = 0,
  lr = 0,
  rl,
  bd,
  numGUIBiDis,
  defaultGUIBiDi = lr
} TGUIBiDi;

#ifdef GUI_DEFINE_OBJECTS
const char* GUIBiDiStrings[] =
{
  "lr",
  "rl",
  "bd",
}; // bidirectional options as strings
#else
extern const char*  GUIBiDiStrings[];
#endif // GUI_DEFINE_OBJECTS

class TGUIRect
{
  public:
    TGUIRect();
    TGUIRect( const TGUIRect& inRect );
    TGUIRect( float inLeft, float inTop, float inRight, float inBottom );

    TPresError ReadFromParam( const Param   *inParamPtr );

    float   left;
    float   top;
    float   right;
    float   bottom;
};

// Presentation elements with an associated color.
// In resource/image files defining styles, the elements appear in the order
// defined here.
typedef enum TGUIElement
{
  firstGUIElement = 0,
  fbBackground = 0,
  visualMarker,
  targetFillNormal,
  targetFillBlinking,
  targetFillActive,
  targetBorderNormal,
  targetBorderBlinking,
  targetBorderActive,
  targetTextNormal,
  targetTextBlinking,
  targetTextActive,
  textFrameBackground,
  scoreViewText,
  numGUIElements
} TGUIElement;

#endif // GUI_H

