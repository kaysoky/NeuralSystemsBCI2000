/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
#ifndef GCC_PREFIX_H
#define GCC_PREFIX_H

#ifndef __GNUC__
# error GCC prefix header file included for a compiler that is not GCC.
#endif

#define NO_PCHINCLUDES
#define __FUNC__        __PRETTY_FUNCTION__
#define pow10( x )      pow( 10., (x) )
#define stricmp         strcasecmp

#endif /* GCC_PREFIX_H */




