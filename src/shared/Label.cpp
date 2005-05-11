////////////////////////////////////////////////////////////////////////////////
//
// File: Label.h
//
// Description: A class that centralizes handling of visualization labels.
//
// Author: juergen.mellinger@uni-tuebingen.de
//
// Date:   May 6, 2005
//
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "Label.h"

using namespace std;

void
Label::WriteToStream( std::ostream& os ) const
{
  os << mAddress << " " << mText;
}

void
Label::ReadFromStream( std::istream& is )
{
  is >> mAddress >> mText;
}
