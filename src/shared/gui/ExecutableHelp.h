////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class that displays an executable's associated
//   help file.
//   The help file is a html file that has the same name as the executable,
//   except that it bears a .html extension.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef EXECUTABLE_HELP_H
#define EXECUTABLE_HELP_H

class ExecutableHelp
{
 public:
  void Display() const;
};

#endif // EXECUTABLE_HELP_H
