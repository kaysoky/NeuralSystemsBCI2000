////////////////////////////////////////////////////////////////////////////////
//
// File: UGenericFilter.h
//
// Description: This file declares a purely abstract GenericFilter interface
//   which all BCI2000 filters are supposed to implement.
//
// Changes: Oct 21, 2002, juergen.mellinger@uni-tuebingen.de
//          - Made GenericFilter a true base class, and a purely abstract one.
//          Mar 19, 2003, juergen.mellinger@uni-tuebingen.de
//          - Removed references to essentially global objects
//            of types CORECOMM, PARAMLIST, STATELIST, STATEVECTOR, and
//            introduced the Environment class to handle access to those objects.
//          - Added the Preflight() member as a purely virtual function to enforce
//            implementation in subclasses.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef UGenericFilterH
#define UGenericFilterH

#include <typeinfo>
#include <set>
#include "UEnvironment.h"
// Includes needed for every filter, so they are put here for
// convenience.
#include "UGenericSignal.h"
#include "UBCIError.h"

// Some utility macros for more readable filter constructors.
#define BEGIN_PARAMETER_DEFINITIONS                                    \
{                                                                      \
  const char* _params[] =                                              \
  {

#define END_PARAMETER_DEFINITIONS                                      \
  };                                                                   \
  for( size_t i = 0; i < sizeof( _params ) / sizeof( *_params ); ++i ) \
    Parameters->AddParameter2List( _params[ i ] );                     \
};

#define BEGIN_STATE_DEFINITIONS                                        \
{                                                                      \
  const char* _states[] =                                              \
  {

#define END_STATE_DEFINITIONS                                          \
  };                                                                   \
  for( size_t i = 0; i < sizeof( _states ) / sizeof( *_states ); ++i ) \
    States->AddState2List( _states[ i ] );                             \
};

class GenericFilter : protected Environment
{
 friend class FILTERS;
 friend class Documentar;

 public:
          GenericFilter()
          { filters.insert( this ); }
  virtual ~GenericFilter()
          { filters.erase( this ); }
  virtual void Preflight( const SignalProperties& Input,
                                SignalProperties& Output ) const = 0;
  virtual void Initialize() = 0;
  virtual void Process( const GenericSignal* Input,
                              GenericSignal* Output ) = 0;
  virtual void Resting() {}
  virtual void Halt() {}

 // The following elements provide means to make the existence of a filter
 // class known to the framework at runtime, without the need to make changes
 // in framework code.
 // A registrar is an auto-created class known to the framework code
 // that allows for instantiation of its associated class without knowledge
 // of the associated class' name etc.
 // A separate Registrar class per filter -- as opposed to a static instance
 // of each filter in the filter's cpp file -- is necessary because
 // - the standard puts functionality inside the GenericFilter constructor
 //   that requires more than is granted at initialization time, and
 // - we don't want to restrict ourselves to a single instance per filter.
 //
 // To use the mechanism, all that is required is a line that says
 //   RegisterFilter( MyFilterClass, FilterPosition );
 // somewhere in the filter's cpp file where FilterPosition is a
 // symbol that, by string comparison, will determine the filter's
 // position in the filter sequence.
 private:
  class Registrar
  {
   public:
    Registrar( const char* p ) : pos( p ), instance( createdInstances++ )
    { registrars.insert( this ); }
    virtual ~Registrar()
    { registrars.erase( this ); }
    const std::string& GetPosition() const { return pos; }
    virtual const std::type_info& GetTypeid() const = 0;
    virtual GenericFilter* NewInstance() const = 0;

   private:
    std::string pos;

   public:
    struct less;
    friend struct less;
    struct less
    {
      bool operator() ( const Registrar* a, const Registrar* b )
      { return ( a->pos == b->pos ) ? ( a->instance < b->instance ) : ( a->pos < b->pos ); }
    };

    typedef std::set<Registrar*, Registrar::less> _registrarSet;
    static _registrarSet registrars;

   private:
    size_t instance;
    static size_t createdInstances;
  };
  typedef Registrar::_registrarSet registrarSet;


 public:
  // Register a filter for automatic instantiation.
  template<typename T> class FilterRegistrar : public Registrar
  {
   public:
    FilterRegistrar( const char* p ) : Registrar( p ) {}
    virtual const std::type_info& GetTypeid() const
    { return typeid( T ); }
    virtual GenericFilter* NewInstance() const
    { return new T; }
  };
  #define RegisterFilter( name, pos )  GenericFilter::FilterRegistrar<name> name##Registrar(#pos);

  // Instantiate all registered filters once.
  static void InstantiateFilters();
  // Create a new instance of the same type as the argument pointer.
  static GenericFilter* NewInstance( const GenericFilter* );
  // Get the first filter instance of a given type, e.g.:
  // MyFilter* myFilter = GenericFilter::GetFilter<MyFilter>();
  template<typename T> static T* GetFilter()
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



