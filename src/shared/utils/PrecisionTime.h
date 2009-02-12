/////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: A class wrapper for the system's high precision timer.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
/////////////////////////////////////////////////////////////////////////////
#ifndef PRECISION_TIME_H
#define PRECISION_TIME_H

class PrecisionTime
{
 public:
  typedef unsigned short NumType;

  PrecisionTime()
    : mValue( 0 ) {}
  PrecisionTime( NumType value )
    : mValue( value ) {}

  NumType operator-( PrecisionTime subtrahend ) const
    { return TimeDiff( subtrahend, *this ); }
  operator NumType() const
    { return mValue; }

  static PrecisionTime Now();
  static NumType TimeDiff( NumType, NumType );
  static int     SignedDiff( NumType, NumType );

 private:
  NumType mValue;
};

#endif // PRECISION_TIME_H
