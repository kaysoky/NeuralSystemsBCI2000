////////////////////////////////////////////////////////////////////////////////
//
// File: UGenericFilter.h
//
// Description: This file declares a purely abstract GenericFilter interface
//   which all BCI2000 filters are supposed to implement.
//
// Changes: Oct 21, 2002, juergen.mellinger@uni-tuebingen.de
//          - Made GenericFilter a true base class, and a purely abstract one.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef UGenericFilterH
#define UGenericFilterH

//#define GF_FINAL

class PARAMLIST;
class STATELIST;
class STATEVECTOR;
class GenericSignal;
#ifdef GF_FINAL
class SignalProperties;
#endif
class CORECOMM;


class GenericFilter
{
 public:
#ifndef GF_FINAL
          GenericFilter()
          : statevector( NULL ), corecomm( NULL ) {}
#endif
  virtual ~GenericFilter() {}

#ifdef GF_FINAL
  virtual void Initialize( PARAMLIST*, STATEVECTOR*, CORECOMM* ) = 0;
#else
  virtual void Initialize( PARAMLIST*, STATEVECTOR*, CORECOMM* ) {}
#endif

#ifdef GF_FINAL
  virtual void Process( const GenericSignal* Input,
                              GenericSignal* Output ) = 0;
#else
  virtual void Process( const GenericSignal* Input,
                              GenericSignal* Output ) {}
#endif

#ifdef GF_FINAL
  virtual void Preflight( const SignalProperties* Input,
                                SignalProperties* Output ) = 0;
#endif

#ifndef GF_FINAL
 protected:
  STATEVECTOR* statevector;
  CORECOMM*    corecomm;
#endif
};

#endif // UGenericFilterH



