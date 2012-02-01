////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A window that contains formatted text.
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
#ifndef TEXT_WINDOW_H
#define TEXT_WINDOW_H

#include <string>

#if __BORLANDC__
# include <VCL.h>
#else // __BORLANDC__
# include <QtGui>
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
  float FontSize() const;

 private:
  mutable std::string mTextBuf;
#if __BORLANDC__
  TForm* mpForm;
  TEdit* mpEditField;
#else // __BORLANDC__
  QWidget*   mpForm;
  QTextEdit* mpEditField;
#endif // __BORLANDC__
};

#endif // TEXT_WINDOW_H

