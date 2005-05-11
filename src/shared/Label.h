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
#ifndef LabelH
#define LabelH

#include <iostream>
#include <string>
#include <vector>
#include "UParameter.h"
#include "VisList.h"

class Label
{
  public:
    Label() : mAddress( 0 ) {}
    Label( int address, const std::string& text ) : mAddress( address ), mText( text ) {}
    virtual ~Label() {}

    int                Address() const  { return mAddress; }
    const std::string& Text() const     { return mText; }

    void WriteToStream( std::ostream& ) const;
    void ReadFromStream( std::istream& );

  private:
    int                  mAddress;
    PARAM::encodedString mText;
};

typedef VisList<Label> Labellist;

inline std::ostream& operator<<( std::ostream& s, const Label& l )
{
  l.WriteToStream( s );
  return s;
}

inline std::istream& operator>>( std::istream& s, Label& l )
{
  l.ReadFromStream( s );
  return s;
}

#endif // LabelH

