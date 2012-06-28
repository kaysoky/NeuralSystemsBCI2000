////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A stimulus consisting of an image.
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

#include "ImageStimulus.h"

#include "FileUtils.h"
#include "BCIError.h"

#ifdef __BORLANDC__
# include <VCL.h>
#else // __BORLANDC__
# include <QPainter>
# include <QBitmap>
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
    mpImage->LoadFromFile( FileUtils::AbsolutePath( inName ).c_str() );
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
  errorOccurred = !mpImage->load( QString( FileUtils::AbsolutePath( inName ).c_str() ) );

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
  if( inMode != mRenderingMode )
  {
    mRenderingMode = inMode;
    Change();
  }
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
    inDC.handle.painter->drawPixmap( int( inDC.rect.left ), int( inDC.rect.top ), *pBuffer );
  }

#endif // __BORLANDC__
}

void
ImageStimulus::OnResize( DrawContext& ioDC )
{
  if( AspectRatioMode() == GUI::AspectRatioModes::AdjustBoth )
    OnMove( ioDC );
  else
    OnChange( ioDC );
}

void
ImageStimulus::OnMove( DrawContext& ioDC )
{
  AdjustRect( ioDC.rect );
}

void
ImageStimulus::OnChange( DrawContext& ioDC )
{
  delete mpImageBufferNormal;
  mpImageBufferNormal = NULL;
  delete mpImageBufferHighlighted;
  mpImageBufferHighlighted = NULL;

  AdjustRect( ioDC.rect );
  int width = ioDC.rect.Width(),
      height = ioDC.rect.Height();

#ifdef __BORLANDC__

  if( mpImage != NULL )
  {
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
    QImage img = mpImage->scaled( width, height );
    // Create the normal pixmap
    if( PresentationMode() != ShowHide )
      mpImageBufferNormal = NewBufferFromImage( img );
    // Create the highlighted pixmap by modifying img
    switch( PresentationMode() )
    {
      case ShowHide:
        delete mpImageBufferNormal;
        mpImageBufferNormal = NULL;
        break;

      case Intensify:
        img = img.convertToFormat( QImage::Format_Indexed8 );
        for( int i = 0; i < img.colorCount(); ++i )
          img.setColor( i, QColor( img.color( i ) ).lighter( static_cast<int>( 100 * DimFactor() ) ).rgb() );
        break;

      case Grayscale:
        img = img.convertToFormat( QImage::Format_Indexed8 );
        for( int i = 0; i < img.colorCount(); ++i )
          img.setColor( i, QColor( img.color( i ) ).value() );
        break;
        break;

      case Invert:
        img.invertPixels();
        break;

      case Dim:
        img = img.convertToFormat( QImage::Format_Indexed8 );
        for( int i = 0; i < img.colorCount(); ++i )
          img.setColor( i, QColor( img.color( i ) ).darker( static_cast<int>( 100 * DimFactor() ) ).rgb() );
        break;
    }
    mpImageBufferHighlighted = NewBufferFromImage( img );
  }

#endif // __BORLANDC__

}

void
ImageStimulus::AdjustRect( GUI::Rect& ioRect ) const
{
  if( mpImage != NULL )
  {
    int width = ioRect.Width(),
        height = ioRect.Height(),
        hCenter = ( ioRect.left + ioRect.right ) / 2,
        vCenter = ( ioRect.bottom + ioRect.top ) / 2;
#ifdef __BORLANDC__
    int imageWidth = mpImage->Width,
        imageHeight = mpImage->Height;
#else // __BORLANDC__
    int imageWidth = mpImage->width(),
        imageHeight = mpImage->height();
#endif // __BORLANDC__

    switch( AspectRatioMode() )
    {
      case GUI::AspectRatioModes::AdjustWidth:
        width = ( imageWidth * height ) / imageHeight;
        ioRect.left = hCenter - width / 2;
        ioRect.right = ioRect.left + width;
        break;

      case GUI::AspectRatioModes::AdjustHeight:
        height = ( imageHeight * width ) / imageWidth;
        ioRect.top = vCenter - height / 2;
        ioRect.bottom = ioRect.top + height;
        break;

      case GUI::AspectRatioModes::AdjustBoth:
        width = imageWidth;
        height = imageHeight;
        ioRect.left = hCenter - width / 2;
        ioRect.right = ioRect.left + width;
        ioRect.top = vCenter - height / 2;
        ioRect.bottom = ioRect.top + height;
        break;

      case GUI::AspectRatioModes::AdjustNone:
      default:
        ;
    }
  }
}


#ifndef __BORLANDC__
QPixmap*
ImageStimulus::NewBufferFromImage( QImage& inImage ) const
{
  QPixmap* pBuffer = new QPixmap( QPixmap::fromImage( inImage ) );
  if( mRenderingMode == GUI::RenderingMode::Transparent && !inImage.hasAlphaChannel() )
  {
    QRgb c = inImage.pixel( 0, 0 );
    QImage mask = inImage.createMaskFromColor( c, Qt::MaskInColor );
    pBuffer->setMask( QBitmap::fromImage( mask ) );
  }
  return pBuffer;
}
#endif // __BORLANDC__

