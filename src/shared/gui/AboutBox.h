////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class that displays a dialog window showing
//   the BCI2000 logo, and versioning information.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef ABOUT_BOX_H
#define ABOUT_BOX_H

#include <string>
#include <map>
#include "VersionInfo.h"

class AboutBox
{
 public:
  AboutBox();
  ~AboutBox()
    {}
  AboutBox& SetApplicationName( const std::string& s )
    { mApplicationName = s; return *this; }
  AboutBox& SetVersionInfo( const std::string& s );
  AboutBox& Display() const;

 private:
  std::string mApplicationName;
  VersionInfo mVersionInfo;
};

#endif // ABOUT_BOX_H
