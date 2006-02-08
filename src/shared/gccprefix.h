#ifndef GCC_PREFIX_H
#define GCC_PREFIX_H

#ifndef __GNUC__
# error GCC prefix header file included for a compiler that is not GCC.
#endif

#define NO_PCHINCLUDES
#define __FUNC__        __PRETTY_FUNCTION__
#define pow10( x )      pow( 10., (x) )

#endif /* GCC_PREFIX_H */
