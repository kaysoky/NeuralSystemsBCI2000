////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A GraphObject displaying a line of text.
//
// $BEGIN_BCI2000_LICENSE$
//
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2012: BCI2000 team and many external contributors ]
//
// BCI2000 is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// BCI2000 is distributed in the hope that it will be useful, but
//                         WITHOUT ANY WARRANTY
// - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <http://www.gnu.org/licenses/>.
//
// $END_BCI2000_LICENSE$
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#ifdef __BORLANDC__
# include "VCL.h"
#else // __BORLANDC__
# include <QFont>
# include <QPainter>
# include <QFontMetrics>
#endif // __BORLANDC__

#include "TextField.h"

using namespace std;
using namespace GUI;

TextField::TextField( GraphDisplay& display, int zOrder )
: GraphObject( display, zOrder ),
  mTextHeight( 1.0 ),
  mColor( RGBColor::NullColor ),
  mTextColor( RGBColor::Gray )
{
}

TextField::~TextField()
{
}

TextField&
TextField::SetText( const string& s )
{
  mText = s;
  Change();
  return *this;
}

const string&
TextField::Text() const
{
  return mText;
}

TextField&
TextField::SetTextHeight( float f )
{
  mTextHeight = f;
  Change();
  return *this;
}

float
TextField::TextHeight() const
{
  return mTextHeight;
}

TextField&
TextField::SetTextColor( RGBColor c )
{
  mTextColor = c;
  Change();
  return *this;
}

RGBColor
TextField::TextColor() const
{
  return mTextColor;
}

TextField&
TextField::SetColor( RGBColor c )
{
  mColor = c;
  Change();
  return *this;
}

RGBColor
TextField::Color() const
{
  return mColor;
}

void
TextField::OnChange( GUI::DrawContext& ioDC )
{
#ifdef __BORLANDC__
  if( ioDC.handle == NULL )
    return;
#else // __BORLANDC__
  if( ioDC.handle.device == NULL )
    return;
#endif // __BORLANDC__

#ifdef __BORLANDC__

  TCanvas* pCanvas = new TCanvas;
  try
  {
    int width = ioDC.rect.right - ioDC.rect.left,
        height = ioDC.rect.bottom - ioDC.rect.top,
        hCenter = ( ioDC.rect.right + ioDC.rect.left ) / 2,
        vCenter = ( ioDC.rect.bottom + ioDC.rect.top ) / 2;

    pCanvas->Handle = ( HDC )ioDC.handle;

    pCanvas->Font->Name = "Arial";
    pCanvas->Font->Height = -mTextHeight * height;
    pCanvas->Font->Style = pCanvas->Font->Style << fsBold;

    AnsiString text = AnsiString( " " ) + mText.c_str() + " ";
    TSize size = pCanvas->TextExtent( text );

    switch( AspectRatioMode() )
    {
      case AspectRatioModes::AdjustWidth:
        width = size.cx;
        break;

      case AspectRatioModes::AdjustHeight:
        height = ( height * width ) / size.cx;
        break;

      case AspectRatioModes::AdjustBoth:
        pCanvas->Font->Height = -mTextHeight;
        size = pCanvas->TextExtent( text );
        height = size.cx;
        width = size.cy;
        break;

      case AspectRatioModes::AdjustNone:
      default:
        ;
    }
    ioDC.rect.left = hCenter - width / 2;
    ioDC.rect.right = hCenter + width / 2;
    ioDC.rect.top = vCenter - height / 2;
    ioDC.rect.bottom = vCenter + height / 2;
  }
  __finally
  {
    delete pCanvas;
  }

#else // __BORLANDC__

  int width = static_cast<int>( ioDC.rect.right - ioDC.rect.left ),
      height = static_cast<int>( ioDC.rect.bottom - ioDC.rect.top ),
      hCenter = static_cast<int>( ( ioDC.rect.right + ioDC.rect.left ) / 2 ),
      vCenter = static_cast<int>( ( ioDC.rect.bottom + ioDC.rect.top ) / 2 ),
      fontSize = static_cast<int>( mTextHeight * height );

  QSize size( 0, 0 );
  if( fontSize > 0 )
  {
    QFont font;
    font.fromString( QString( "Arial" ) );
    font.setPixelSize( fontSize );
    font.setBold( true );
    QFontMetrics fm( font );
    QString text = QString::fromLocal8Bit( mText.c_str() );
    text.append( " " ).prepend( " " );
    size = fm.size( 0, text );
  }

  switch( AspectRatioMode() )
  {
    case AspectRatioModes::AdjustWidth:
      width = size.width();
      break;

    case AspectRatioModes::AdjustHeight:
      height = ( height * width ) / size.width();
      break;

    case AspectRatioModes::AdjustBoth:
      height = size.width();
      width = size.height();
      break;

    case AspectRatioModes::AdjustNone:
    default:
      ;
  }
  ioDC.rect.left = hCenter - width / 2;
  ioDC.rect.right = hCenter + width / 2;
  ioDC.rect.top = vCenter - height / 2;
  ioDC.rect.bottom = vCenter + height / 2;

#endif // __BORLANDC__

  GraphObject::OnChange( ioDC );
}

void
TextField::OnPaint( const GUI::DrawContext& inDC )
{
  DoPaint( inDC, mTextColor, mColor );
}

void
TextField::DoPaint( const GUI::DrawContext& inDC,
                    RGBColor inTextColor,
                    RGBColor inBackgroundColor )
{
#ifdef __BORLANDC__

  TCanvas* pCanvas = new TCanvas;
  try
  {
    pCanvas->Handle = ( HDC )inDC.handle;
    TRect winRect( inDC.rect.left, inDC.rect.top, inDC.rect.right, inDC.rect.bottom );
    if( mColor != RGBColor::NullColor )
    {
      pCanvas->Brush->Style = bsSolid;
      pCanvas->Brush->Color = TColor( inBackgroundColor.ToWinColor() );
      pCanvas->FillRect( winRect );
    }

    pCanvas->Font->Name = "Arial";
    pCanvas->Font->Height = -mTextHeight * ( inDC.rect.bottom - inDC.rect.top );
    pCanvas->Font->Style = pCanvas->Font->Style << fsBold;
    pCanvas->Font->Color = TColor( inTextColor.ToWinColor() );

    AnsiString text = AnsiString( " " ) + mText.c_str() + " ";
    TSize size = pCanvas->TextExtent( text );
    int xPos = winRect.left + ( winRect.Width() - size.cx ) / 2,
        yPos = winRect.top + ( winRect.Height() - size.cy ) / 2;
    pCanvas->Brush->Style = bsClear;
    pCanvas->TextRect( winRect, xPos, yPos, text );
  }
  __finally
  {
    delete pCanvas;
  }

#else // __BORLANDC__

  QPainter* p = inDC.handle.painter;
  QRect rect(
    static_cast<int>( inDC.rect.left ),
    static_cast<int>( inDC.rect.top ),
    static_cast<int>( inDC.rect.right - inDC.rect.left ),
    static_cast<int>( inDC.rect.bottom - inDC.rect.top )
  );
  QBrush brush;
  brush.setStyle( Qt::SolidPattern );
  if( mColor != RGBColor( RGBColor::NullColor ) )
  {
    QColor backColor( mColor.R(), mColor.G(), mColor.B() );
    brush.setColor( backColor );
    p->fillRect( rect, brush );
  }

  QFont font;
  font.fromString( QString( "Arial" ) );
  font.setPixelSize( static_cast<int>( mTextHeight * ( inDC.rect.bottom - inDC.rect.top ) ) );
  font.setBold( true );
  QColor textColor( inTextColor.R(), inTextColor.G(), inTextColor.B() );
  QPen pen;
  brush.setColor( textColor );
  pen.setColor( textColor );
  p->setPen( pen );
  p->setBrush( brush );
  p->setFont( font );

  QString text = QString::fromLocal8Bit( mText.c_str() );
  text.append( " " ).prepend( " " );
  p->drawText( rect, Qt::AlignCenter, text );

#endif // __BORLANDC__
}

