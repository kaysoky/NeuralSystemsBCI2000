////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class that centralizes handling of visualization labels.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef LABEL_H
#define LABEL_H

#include <iostream>
#include <string>
#include <vector>
#include "EncodedString.h"
#include "ValueList.h"

class Label
{
  public:
    Label()
      : mAddress( 0 )
      {}
    Label( int address, const std::string& text )
      : mAddress( address ),
        mText( text )
      {}
    virtual ~Label()
      {}

    int                Address() const
                       { return mAddress; }
    const std::string& Text() const
                       { return mText; }

    std::ostream&      WriteToStream( std::ostream& ) const;
    std::istream&      ReadFromStream( std::istream& );

  private:
    int           mAddress;
    EncodedString mText;
};

typedef ValueList<Label> LabelList;

inline
std::ostream& operator<<( std::ostream& s, const Label& l )
{ return l.WriteToStream( s ); }

inline
std::istream& operator>>( std::istream& s, Label& l )
{ return l.ReadFromStream( s ); }

#endif // LABEL_H

