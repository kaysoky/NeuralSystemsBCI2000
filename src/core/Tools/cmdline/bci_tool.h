////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A small framework for platform independent command
//   line tools.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////
#ifndef BCI_TOOL_H
#define BCI_TOOL_H

#include <iostream>
#include <string>
#include <set>

extern std::string ToolInfo[];
enum ToolInfoIndex
{
  name = 0,
  version,
  short_description,
  description,
  firstOption
};


typedef enum ToolResult
{
  noError = 0,
  illegalOption,
  illegalInput,
  fileIOError,
  genericError
} ToolResult;

class OptionSet : public std::set<std::string>
{
 public:
  std::string getopt( const std::string& optionNames, const std::string& optionDefault ) const;
};

ToolResult ToolInit();
ToolResult ToolMain( const OptionSet&, std::istream&, std::ostream& );

#endif // BCI_TOOL_H
