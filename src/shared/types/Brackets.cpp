////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: This helper class defines what we accept as delimiting
//   single-character symbol pairs for index lists and sub-parameters in a
//   parameter line.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "Brackets.h"

using namespace std;

const string&
Brackets::BracketPairs()
{ // Bracket pairs that are recognized when reading.
  static string bracketPairs = "{}[]";
  return bracketPairs;
}

