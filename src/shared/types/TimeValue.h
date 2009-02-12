////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class that wraps an integer value such that it reads/writes
//   itself in hexagesimal units.
//   For reading, the following formats are transparently supported:
//      hh:mm:ss (this is also used for writing)
//      mm:ss
//      plain seconds.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef TIME_VALUE_H
#define TIME_VALUE_H

#include <iostream>

class TimeValue
{
 public:
  TimeValue( long t = 0 ) : mValue( t ) {}
  operator long&()
    { return mValue; }
  std::ostream& WriteToStream( std::ostream& os ) const
    {
      int hours = mValue / 3600,
          mins = ( mValue - 3600 * hours ) / 60,
          secs = mValue % 60;
      return os << std::setw( 2 ) << std::setfill( '0' ) << hours
                << ':' << std::setw( 2 ) << mins
                << ':' << std::setw( 2 ) << secs;
    }
  std::istream& ReadFromStream( std::istream& is )
    {
      int val1 = 0,
          val2 = 0,
          val3 = 0;
      if( is >> val1 )
      {
        if( is.peek() == ':' )
        {
          if( is.ignore() >> val2 )
          {
            if( is.peek() == ':' )
            {
              if( is.ignore() >> val3 ) // format is hh:mm:ss
                mValue = 3600 * val1 + 60 * val2 + val3;
            }
            else // format is mm:ss
              mValue = 60 * val1 + val2;
          }
        }
        else // format is plain seconds
          mValue = val1;
      }
      return is;
    }
 private:
  long mValue;
};

inline
std::ostream& operator<<( std::ostream& os, const TimeValue& t )
{ return t.WriteToStream( os ); }

inline
std::istream& operator>>( std::istream& is, TimeValue& t )
{ return t.ReadFromStream( is ); }

#endif // TIME_VALUE_H

