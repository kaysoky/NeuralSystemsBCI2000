////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A stimulus consisting of an image.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "ImageStimulus.h"

#include "BCIDirectory.h"
#include "BCIError.h"
#ifdef __BORLANDC__
# include <VCL.h>
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
#endif // __BORLANDC__
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
      pCanvas->Handle = inDC.handle;
      pCanvas->Draw( inDC.rect.left, inDC.rect.top, pBuffer );
    }
    __finally
    {
      delete pCanvas;
    }
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
        ioDC.rect.right = hCenter + width / 2;
        break;

      case GUI::AspectRatioModes::AdjustHeight:
        height = ( mpImage->Height * width ) / mpImage->Width;
        ioDC.rect.top = vCenter - height / 2;
        ioDC.rect.bottom = vCenter + height / 2;
        break;

      case GUI::AspectRatioModes::AdjustBoth:
        ioDC.rect.left = hCenter - mpImage->Width / 2;
        ioDC.rect.right = hCenter + mpImage->Width / 2;
        ioDC.rect.top = vCenter - mpImage->Height / 2;
        ioDC.rect.bottom = vCenter + mpImage->Height / 2;
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
#endif // __BORLANDC__

  GraphObject::OnChange( ioDC );
}


