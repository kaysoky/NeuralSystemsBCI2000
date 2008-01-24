////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A stimulus consisting of an image.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef IMAGE_STIMULUS_H
#define IMAGE_STIMULUS_H

#include "VisualStimulus.h"
#include "GraphObject.h"

class ImageStimulus : public VisualStimulus, public GUI::GraphObject
{
 public:
  ImageStimulus( GUI::GraphDisplay& display );
  virtual ~ImageStimulus();
  // Properties
  ImageStimulus& SetFile( const std::string& );
  const std::string& File() const;
  ImageStimulus& SetRenderingMode( int );
  int RenderingMode() const;

 protected:
  // GraphObject event handlers
  virtual void OnPaint( const GUI::DrawContext& );
  virtual void OnChange( GUI::DrawContext& );

 private:
  std::string mFile;
  int         mRenderingMode;
#ifdef __BORLANDC__
  TPicture* mpImage;
  Graphics::TBitmap*  mpImageBufferNormal,
                   *  mpImageBufferHighlighted;
#endif // __BORLANDC__
};

#endif // IMAGE_STIMULUS_H

