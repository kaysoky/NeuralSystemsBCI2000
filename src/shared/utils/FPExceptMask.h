//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: An object of this class disables FPU exceptions
//      while it exists, and restores the previous state on
//      destruction.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
///////////////////////////////////////////////////////////////////////
#ifndef FP_EXCEPT_MASK_H
#define FP_EXCEPT_MASK_H

#include <float.h>

#if __BORLANDC__
# define _MCW_EM        MCW_EM
# define _EM_INVALID    EM_INVALID
# define _EM_DENORMAL   EM_DENORMAL
# define _EM_ZERODIVIDE EM_ZERODIVIDE
# define _EM_OVERFLOW   EM_OVERFLOW
# define _EM_UNDERFLOW  EM_UNDERFLOW
# define _EM_INEXACT    EM_INEXACT
#endif // __BORLANDC__

class FPExceptMask
{
 public:
  FPExceptMask( int inExceptions = _MCW_EM )
    : mPrevState( 0 )
    {
      mPrevState = ::_controlfp( _MCW_EM, inExceptions );
    }

  virtual ~FPExceptMask()
    {
      ::_controlfp( _MCW_EM, mPrevState );
    }

 private:
  int mPrevState;
};

#endif // FP_EXCEPT_MASK_H
