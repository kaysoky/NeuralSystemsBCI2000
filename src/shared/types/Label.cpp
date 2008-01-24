////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class that centralizes handling of visualization labels.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "Label.h"

using namespace std;

ostream&
Label::WriteToStream( ostream& os ) const
{
  return os << mAddress << " " << mText;
}

istream&
Label::ReadFromStream( istream& is )
{
  return is >> mAddress >> mText;
}
