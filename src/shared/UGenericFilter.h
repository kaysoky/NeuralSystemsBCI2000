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

#include <typeinfo>
#include <set>

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
          GenericFilter()
          : statevector( NULL ), corecomm( NULL )
          { filters.insert( this ); }
  virtual ~GenericFilter()
          { filters.erase( this ); }
  virtual void Initialize( PARAMLIST*, STATEVECTOR*, CORECOMM* ) = 0;
  virtual void Process( const GenericSignal* Input,
                              GenericSignal* Output ) = 0;

#ifdef GF_FINAL
  virtual void Preflight( const SignalProperties* Input,
                                SignalProperties* Output ) = 0;
#endif

 protected:
#ifndef GF_FINAL
  STATEVECTOR* statevector;
  CORECOMM*    corecomm;
#endif
  // Get a filter instance of a given type, e.g.:
  // MyFilter* myFilter = GenericFilter::GetFilter<MyFilter>();
  //
  // This is a workaround for adapting legacy code. Don't use it in new code.
  // It ignores the possibility of multiple instances of the same filter class.
  //
  template<class T> static T* GetFilter()
  {
    T* filterFound = NULL;
    filterSet::iterator i = filters.begin();
    while( i != filters.end() && filterFound == NULL )
    {
      filterFound = dynamic_cast<T*>( *i );
      ++i;
    }
    return filterFound;
  }

 private:
  typedef std::set<GenericFilter*> filterSet;
  static filterSet filters;
};

#endif // UGenericFilterH



