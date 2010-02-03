////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A GraphObject consisting of two stacked text fields
//   separated with a separator bar.
//   GoalText is the top field, ResultText is the bottom field.
//   For an empty GoalText, ResultText occupies the top as well, and the
//   separator bar is not displayed.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "StatusBar.h"

using namespace std;
using namespace GUI;

StatusBar::StatusBar( GraphDisplay& display )
: GraphObject( display, StatusBarZOrder ),
  mTextHeight( 1.0 ),
  mColor( RGBColor::NullColor ),
  mTextColor( RGBColor::Gray )
{
}

StatusBar::~StatusBar()
{
}

StatusBar&
StatusBar::SetGoalText( const string& s )
{
  mGoalText = s;
  Change();
  return *this;
}

const string& StatusBar::GoalText() const
{
  return mGoalText;
}

StatusBar&
StatusBar::SetResultText( const string& s )
{
  mResultText = s;
  Change();
  return *this;
}

const string& StatusBar::ResultText() const
{
  return mResultText;
}

StatusBar&
StatusBar::SetTextHeight( float f )
{
  mTextHeight = f;
  Change();
  return *this;
}

float
StatusBar::TextHeight() const
{
  return mTextHeight;
}

StatusBar&
StatusBar::SetTextColor( RGBColor c )
{
  mTextColor = c;
  Change();
  return *this;
}

RGBColor
StatusBar::TextColor() const
{
  return mTextColor;
}

StatusBar&
StatusBar::SetColor( RGBColor c )
{
  mColor = c;
  Change();
  return *this;
}

RGBColor
StatusBar::Color() const
{
  return mColor;
}

void
StatusBar::OnPaint( const DrawContext& dc )
{
  const int heightRatioGoal = 1,
            heightRatioResult = 1;
  RGBColor  dividerColor = RGBColor::Black,
            backgroundColor = mColor,
            textColor = mTextColor;
  string    line1, line2;

  if( mGoalText.empty() )
  {
    size_t spacePos = mResultText.rfind( ' ' );
    if( spacePos != string::npos )
    {
      line1 = mResultText.substr( 0, spacePos );
      line2 = mResultText.substr( spacePos + 1, string::npos );
    }
    else
    {
      line1 = "";
      line2 = mResultText;
    }
  }
  else
  {
    line1 = mGoalText;
    line2 = mResultText;
  }

#ifdef __BORLANDC__
  TCanvas* pCanvas = new TCanvas;
  try
  {
    pCanvas->Handle = dc.handle;
    TRect winRect( dc.rect.left, dc.rect.top, dc.rect.right, dc.rect.bottom );
    if( backgroundColor != RGBColor::NullColor )
    {
      pCanvas->Brush->Style = bsSolid;
      pCanvas->Brush->Color = TColor( backgroundColor.ToWinColor() );
      pCanvas->FillRect( winRect );
    }
    pCanvas->Font->Name = "Arial";
    pCanvas->Font->Height = -mTextHeight * ( dc.rect.bottom - dc.rect.top );
    pCanvas->Font->Style = TFontStyles() << fsBold;
    pCanvas->Font->Color = TColor( textColor.ToWinColor() );
    int charWidth = pCanvas->TextWidth( "x" );

    int dividerPos = winRect.top + ( winRect.Height() * heightRatioGoal )
                                  / ( heightRatioGoal + heightRatioResult );
    if( !mGoalText.empty() )
    {
      TRect dividerRect = winRect;
      dividerRect.top = dividerPos;
      dividerRect.bottom = dividerPos + 1;
      pCanvas->Brush->Color = TColor( dividerColor.ToWinColor() );
      pCanvas->FillRect( dividerRect );
    }

    AnsiString text = line1.c_str();
    TRect line1Rect = winRect;
    line1Rect.bottom = dividerPos;
    TSize size = pCanvas->TextExtent( text );
    while( size.cx > line1Rect.Width() - 2 * charWidth )
    {
      text = text.SubString( 2, text.Length() );
      size = pCanvas->TextExtent( text );
    }
    int xPos = line1Rect.left + charWidth,
        yPos = line1Rect.top + ( line1Rect.Height() - size.cy ) / 2;
    pCanvas->Brush->Style = bsClear;
    pCanvas->TextRect( line1Rect, xPos, yPos, text );

    text = line2.c_str();
    TRect line2Rect = winRect;
    line2Rect.top = dividerPos;
    size = pCanvas->TextExtent( text );
    while( size.cx > line2Rect.Width() - 2 * charWidth )
    {
      text = text.SubString( 2, text.Length() );
      size = pCanvas->TextExtent( text );
    }
    xPos = line2Rect.left + charWidth;
    yPos = line2Rect.top + ( line2Rect.Height() - size.cy ) / 2;
    pCanvas->Brush->Style = bsClear;
    pCanvas->TextRect( line2Rect, xPos, yPos, text );
  }
  __finally
  {
    delete pCanvas;
  }
#endif // __BORLANDC__
}

