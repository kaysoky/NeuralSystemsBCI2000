////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class for bitmap type visualization windows.
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

#include "VisDisplayBitmap.h"

#include <QPaintEvent>
#include <QMouseEvent>
#include <QPainter>

#ifdef _WIN32
# include <windows.h>
#endif // _WIN32

using namespace std;

VisDisplayBitmap::VisDisplayBitmap( const std::string& inSourceID )
: VisDisplayLayer( inSourceID ),
  mWaitForReferenceFrame( false )
{
  Restore();
}

VisDisplayBitmap::~VisDisplayBitmap()
{
  Save();
}

void
VisDisplayBitmap::SetConfig( ConfigSettings& inConfig )
{
  VisDisplayBase::SetConfig( inConfig );
}

void
VisDisplayBitmap::Restore()
{
  VisDisplayBase::Restore();
  this->show();
}

void
VisDisplayBitmap::Save() const
{
  VisDisplayBase::Save();
}

void
VisDisplayBitmap::HandleBitmap( const BitmapImage& inImage )
{
  if( inImage.Empty() )
  {
    mWaitForReferenceFrame = true;
    return;
  }

  if( mWaitForReferenceFrame )
  {
    if( ( mImageBuffer.Width() != inImage.Width() ) || ( mImageBuffer.Height() != inImage.Height() ) )
    {
      // Adapt the window's aspect ratio without changing its width.
      if( this->hasFocus() && inImage.Width() > 0 )
      {
        QWidget* pParent = this->parentWidget();
        pParent->resize( pParent->width(), ( inImage.Height() * pParent->width() ) / inImage.Width() );
        Visconfigs()[ mVisID.WindowID() ].Put( CfgID::Height, pParent->height(), UserDefined );
      }
    }
    mImageBuffer = inImage;
    mWaitForReferenceFrame = false;
  }
  else
  {
    mImageBuffer += inImage;
  }
  this->update();
}

void
VisDisplayBitmap::paintEvent( QPaintEvent* iopEvent )
{
  QPainter p( this );
  p.setRenderHint( QPainter::Antialiasing, false );
  p.setRenderHint( QPainter::TextAntialiasing, false );

  int formWidth = this->width(),
      formHeight = this->height();
  if( !mImageBuffer.Empty() )
  {
    for( int x = 0; x < mImageBuffer.Width(); ++x )
    {
      for( int y = 0; y < mImageBuffer.Height(); ++y )
      {
        RGBColor rgb = mImageBuffer( x, y );
        if( rgb != RGBColor::NullColor )
        {
          QRect pixelRect;
          pixelRect.setLeft( ( x * formWidth ) / mImageBuffer.Width() );
          pixelRect.setTop( ( y * formHeight ) / mImageBuffer.Height() );
          pixelRect.setRight( ( ( x + 1 ) * formWidth ) / mImageBuffer.Width() );
          pixelRect.setBottom( ( ( y + 1 ) * formHeight ) / mImageBuffer.Height() );
          p.fillRect( pixelRect, QColor( rgb.R(), rgb.G(), rgb.B() ) );
        }
      }
    }
#ifdef _WIN32
    ::Sleep( 0 );
#endif // _WIN32
  }
  VisDisplayBase::paintEvent( iopEvent );
  iopEvent->accept();
}

void
VisDisplayBitmap::mousePressEvent( QMouseEvent* iopEvent )
{
  this->parentWidget()->activateWindow();
  VisDisplayBase::mousePressEvent( iopEvent );
}
