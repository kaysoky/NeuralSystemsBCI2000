//---------------------------------------------------------------------------

#ifndef UBCItimeH
#define UBCItimeH
//---------------------------------------------------------------------------

class BCITIME
{
private:
  unsigned short value;

public:		// User declarations
  BCITIME();
  BCITIME( unsigned short );
  BCITIME operator-( BCITIME ) const;
  operator unsigned short() const;

  static unsigned short GetBCItime_ms();
  static unsigned short TimeDiff( unsigned short time1, unsigned short time2 );
};

// **************************************************************************
// Function:   BCITIME
// Purpose:    a constructor for the BCITIME object
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
inline
BCITIME::BCITIME()
: value( 0 )
{
}

// **************************************************************************
// Function:   BCITIME
// Purpose:    a constructor for the BCITIME object
// Parameters: inValue - an unsigned short integer
// Returns:    N/A
// **************************************************************************
inline
BCITIME::BCITIME( unsigned short inValue )
: value( inValue )
{
}

// **************************************************************************
// Function:   operator unsigned short
// Purpose:    a typecast operator to unsigned short
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
inline
BCITIME::operator unsigned short() const
{
  return value;
}

// **************************************************************************
// Function:   operator-
// Purpose:    an operator wrapper for the TimeDiff function
// Parameters: timeToSubtract - a second BCITIME object
// Returns:    the result of the call to TimeDiff
// **************************************************************************
inline
BCITIME
BCITIME::operator-( BCITIME timeToSubtract ) const
{
  return TimeDiff( timeToSubtract, *this );
}
#endif

