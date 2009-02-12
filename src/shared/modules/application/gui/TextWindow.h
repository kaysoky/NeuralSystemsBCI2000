////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A window that contains formatted text.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef TEXT_WINDOW_H
#define TEXT_WINDOW_H

#include <string>

#ifdef __BORLANDC__
# include <VCL.h>
#endif // __BORLANDC__

class TextWindow
{
 public:
  TextWindow();
  virtual ~TextWindow();

  // Properties
  TextWindow& Show();
  TextWindow& Hide();

  TextWindow& SetLeft( int );
  int Left() const;
  TextWindow& SetTop( int );
  int Top() const;
  TextWindow& SetWidth( int );
  int Width() const;
  TextWindow& SetHeight( int );
  int Height() const;

  TextWindow& SetText( const std::string& );
  const std::string& Text() const;

  TextWindow& SetFontName( const std::string& );
  const std::string& FontName() const;
  TextWindow& SetFontSize( int );
  int FontSize() const;

 private:
  mutable std::string mTextBuf;
#ifdef __BORLANDC__
  TForm* mpForm;
  TEdit* mpEditField;
#endif // __BORLANDC__
};

#endif // TEXT_WINDOW_H

