////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A stimulus consisting of an image.
//
// $BEGIN_BCI2000_LICENSE$
// 
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2011: BCI2000 team and many external contributors ]
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

#include "ImageStimulus.h"

#include "BCIDirectory.h"
#include "BCIError.h"

#ifdef __BORLANDC__
# include <VCL.h>
#else // __BORLANDC__
# include <QPainter>
#endif // __BORLANDC__

using namespace std;
using namespace GUI;

ImageStimulus::ImageStimulus( GraphDisplay& display )
: GraphObject( display, ImageStimulusZOrder ),
  mRenderingMode( RenderingMode::Opaque ),
  mpImage( NULL ),
  mpImageBufferNormal( NULL ),
  mpImageBufferHighlighted( NULL )
{
}

ImageStimulus::~ImageStimulus()
{
  delete mpImage;
  delete mpImageBufferNormal;
  delete mpImageBufferHighlighted;
}

ImageStimulus&
ImageStimulus::SetFile( const string& inName )
{
  bool errorOccurred = false;

  // Attempt to load the image
#ifdef __BORLANDC__

  delete mpImage;
  mpImage = new TPicture;
  try
  {
    mpImage->LoadFromFile( BCIDirectory::AbsolutePath( inName ).c_str() );
  }
  catch( EInOutError& )
  {
    errorOccurred = true;
  }
  catch( EFOpenError& )
  {
    errorOccurred = true;
  }
  catch( EInvalidGraphic& )
  {
    errorOccurred = true;
  }

#else // __BORLANDC__

  delete mpImage;
  mpImage = new QImage();
  errorOccurred = !mpImage->load( QString( BCIDirectory::AbsolutePath( inName ).c_str() ) );

#endif // __BORLANDC__

  // An error occurred while loading the image
  if( errorOccurred )
  {
    bcierr << "Could not load image from file \"" << inName << "\"" << endl;
    delete mpImage;
    mpImage = NULL;
    mFile = "";
  }
  else
    mFile = inName;

  Change();
  return *this;
}

const string&
ImageStimulus::File() const
{
  return mFile;
}

ImageStimulus&
ImageStimulus::SetRenderingMode( int inMode )
{
  mRenderingMode = inMode;
  Invalidate();
  return *this;
}

int
ImageStimulus::RenderingMode() const
{
  return mRenderingMode;
}

void
ImageStimulus::OnPaint( const DrawContext& inDC )
{
  // Draw the proper buffered image using the given DrawContext
#ifdef __BORLANDC__

  Graphics::TBitmap* pBuffer = BeingPresented() ?
                               mpImageBufferHighlighted :
                               mpImageBufferNormal;
  if( pBuffer != NULL )
  {
    TCanvas* pCanvas = new TCanvas;
    try
    {
      pBuffer->Transparent = ( mRenderingMode == GUI::RenderingMode::Transparent );
      pBuffer->TransparentMode = tmAuto;
      pCanvas->Handle = ( HDC )inDC.handle;
      pCanvas->Draw( inDC.rect.left, inDC.rect.top, pBuffer );
    }
    __finally
    {
      delete pCanvas;
    }
  }

#else // __BORLANDC__

  QPixmap* pBuffer = BeingPresented() ?
                    mpImageBufferHighlighted :
                    mpImageBufferNormal;
  if( pBuffer != NULL )
  {
    QPainter painter( inDC.handle );
    painter.drawPixmap( int( inDC.rect.left ), int( inDC.rect.top ), *pBuffer );
  }

#endif // __BORLANDC__
}

void
ImageStimulus::OnChange( DrawContext& ioDC )
{
  delete mpImageBufferNormal;
  mpImageBufferNormal = NULL;
  delete mpImageBufferHighlighted;
  mpImageBufferHighlighted = NULL;

#ifdef __BORLANDC__

  if( mpImage != NULL )
  {
    int width = ioDC.rect.right - ioDC.rect.left,
        height = ioDC.rect.bottom - ioDC.rect.top,
        hCenter = ( ioDC.rect.left + ioDC.rect.right ) / 2,
        vCenter = ( ioDC.rect.bottom + ioDC.rect.top ) / 2;

    switch( AspectRatioMode() )
    {
      case GUI::AspectRatioModes::AdjustWidth:
        width = ( mpImage->Width * height ) / mpImage->Height;
        ioDC.rect.left = hCenter - width / 2;
        ioDC.rect.right = ioDC.rect.left + width;
        break;

      case GUI::AspectRatioModes::AdjustHeight:
        height = ( mpImage->Height * width ) / mpImage->Width;
        ioDC.rect.top = vCenter - height / 2;
        ioDC.rect.bottom = ioDC.rect.top + height;
        break;

      case GUI::AspectRatioModes::AdjustBoth:
        width = mpImage->Width;
        height = mpImage->Height;
        ioDC.rect.left = hCenter - width / 2;
        ioDC.rect.right = ioDC.rect.left + width;
        ioDC.rect.top = vCenter - height / 2;
        ioDC.rect.bottom = ioDC.rect.top + height;
        break;

      case GUI::AspectRatioModes::AdjustNone:
      default:
        ;
    }
    TRect bufRect( 0, 0, width, height );
    mpImageBufferNormal = new Graphics::TBitmap;
    mpImageBufferNormal->Width = width;
    mpImageBufferNormal->Height = height;
    mpImageBufferNormal->Canvas->StretchDraw( bufRect, mpImage->Graphic );

    mpImageBufferHighlighted = new Graphics::TBitmap;
    mpImageBufferHighlighted->Assign( mpImageBufferNormal );

    TCanvas* pCanvas = mpImageBufferHighlighted->Canvas;
    switch( PresentationMode() )
    {
      case ShowHide:
        delete mpImageBufferNormal;
        mpImageBufferNormal = NULL;
        break;

      case Intensify:
        for( int i = 0; i < bufRect.Width(); ++i )
          for( int j = 0; j < bufRect.Height(); ++j )
          {
            RGBColor c = RGBColor::FromWinColor( pCanvas->Pixels[ i ][ j ] );
            c *= 1 / DimFactor();
            pCanvas->Pixels[ i ][ j ] = TColor( c.ToWinColor() );
          }
        break;

      case Grayscale:
        mpImageBufferHighlighted->Monochrome = true;
        break;

      case Invert:
        for( int i = 0; i < bufRect.Width(); ++i )
          for( int j = 0; j < bufRect.Height(); ++j )
            pCanvas->Pixels[ i ][ j ] = TColor( ~pCanvas->Pixels[ i ][ j ] );
        break;

      case Dim:
        for( int i = 0; i < bufRect.Width(); ++i )
          for( int j = 0; j < bufRect.Height(); ++j )
          {
            RGBColor c = RGBColor::FromWinColor( pCanvas->Pixels[ i ][ j ] );
            c *= DimFactor();
            pCanvas->Pixels[ i ][ j ] = TColor( c.ToWinColor() );
          }
        break;
    }
  }

#else // __BORLANDC__

  if( mpImage != NULL )
  {
    int imageWidth = static_cast<int>( ioDC.rect.right - ioDC.rect.left ),
        imageHeight = static_cast<int>( ioDC.rect.bottom - ioDC.rect.top ),
        hCenter = static_cast<int>( ( ioDC.rect.left + ioDC.rect.right ) / 2 ),
        vCenter = static_cast<int>( ( ioDC.rect.bottom + ioDC.rect.top ) / 2 );

    switch( AspectRatioMode() )
    {
      case GUI::AspectRatioModes::AdjustWidth:
        imageWidth = ( mpImage->width() * imageHeight ) / mpImage->height();
        ioDC.rect.left = hCenter - imageWidth / 2;
        ioDC.rect.right = ioDC.rect.left + imageWidth;
        break;

      case GUI::AspectRatioModes::AdjustHeight:
        imageHeight = ( mpImage->height() * imageWidth ) / mpImage->width();
        ioDC.rect.top = vCenter - imageHeight / 2;
        ioDC.rect.bottom = ioDC.rect.top + imageHeight;
        break;

      case GUI::AspectRatioModes::AdjustBoth:
        ioDC.rect.left = hCenter - mpImage->width() / 2;
        ioDC.rect.right = ioDC.rect.left + mpImage->width();
        ioDC.rect.top = vCenter - mpImage->height() / 2;
        ioDC.rect.bottom = ioDC.rect.top + mpImage->height();
        break;

      case GUI::AspectRatioModes::AdjustNone:
      default:
        ;
    }

    // Create the normal pixmap
    mpImageBufferNormal = new QPixmap();
    *mpImageBufferNormal = QPixmap::fromImage( *mpImage );

    // Create the highlighted pixmap by modifying mpImage
    mpImageBufferHighlighted = new QPixmap();
    QImage temp = *mpImage;
    switch( PresentationMode() )
    {
      case ShowHide:
        delete mpImageBufferNormal;
        mpImageBufferNormal = NULL;
        break;

      case Intensify:
        for( int i = 0; i < mpImage->width(); ++i )
          for( int j = 0; j < mpImage->height(); ++j )
          {
            QColor c = mpImage->pixel( i, j );
            c = c.lighter( static_cast<int>( 100 * DimFactor() ) );
            mpImage->setPixel( i, j, c.rgb() );
          }
        break;

      case Grayscale:
        // May need changing.  Mono makes this monochromatic, not grayscale.
        mpImage->convertToFormat( QImage::Format_Mono );
        break;

      case Invert:
        mpImage->invertPixels();
        break;

      case Dim:
        for( int i = 0; i < mpImage->width(); ++i )
          for( int j = 0; j < mpImage->height(); ++j )
          {
            QColor c = mpImage->pixel( i, j );
            c = c.darker( static_cast<int>( 100 * DimFactor() ) );
            mpImage->setPixel( i, j, c.rgb() );
          }
        break;
    }
    *mpImageBufferHighlighted = QPixmap::fromImage( *mpImage );
    *mpImage = temp;

    // Scale the pixmaps if necessary
    if( mpImageBufferNormal )
      *mpImageBufferNormal = mpImageBufferNormal->scaled( imageWidth, imageHeight );
    if( mpImageBufferHighlighted )
      *mpImageBufferHighlighted = mpImageBufferHighlighted->scaled( imageWidth, imageHeight );
  }

#endif // __BORLANDC__

  GraphObject::OnChange( ioDC );
}


