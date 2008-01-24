////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A GraphObject consisting of two stacked text fields
//   separated with a separator bar.
//   GoalText is the top field, ResultText is the bottom field.
//   For an empty GoalText, ResultText occupies the top as well, and the
//   separator bar is not displayed.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef STATUS_BAR_H
#define STATUS_BAR_H

#include "GraphObject.h"

#include "Color.h"
#include <string>

class StatusBar : public GUI::GraphObject
{
 public:
  StatusBar( GUI::GraphDisplay& display );
  virtual ~StatusBar();
  // Properties
  StatusBar& SetGoalText( const std::string& );
  const std::string& GoalText() const;
  StatusBar& SetResultText( const std::string& );
  const std::string& ResultText() const;
  StatusBar& SetTextHeight( float );
  float TextHeight() const;
  StatusBar& SetTextColor( RGBColor );
  RGBColor TextColor() const;
  StatusBar& SetColor( RGBColor );
  RGBColor Color() const;

 private:
  // GraphObject event handlers
  virtual void OnPaint( const GUI::DrawContext& );

 private:
  std::string mGoalText,
              mResultText;
  float       mTextHeight;
  RGBColor    mColor,
              mTextColor;
};

#endif // STATUS_BAR_H

