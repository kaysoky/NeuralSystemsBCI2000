////////////////////////////////////////////////////////////////////////////////
// $Id$
//
// File: UGenericFilter.h
//
// Changes: Oct 21, 2002, juergen.mellinger@uni-tuebingen.de
//          - Made GenericFilter a true base class, and a purely abstract one.
//          Mar 19, 2003, juergen.mellinger@uni-tuebingen.de
//          - Removed references to essentially global objects
//            of types CORECOMM, PARAMLIST, STATELIST, STATEVECTOR, and
//            introduced the Environment class to handle access to those objects.
//          - Added the Preflight() member as a purely virtual function to enforce
//            implementation in subclasses.
//          Jun 10, 2004, juergen.mellinger@uni-tuebingen.de
//          - Disabled auto-instantiation for signal processing modules in favor
//            of a list of Filter() statements in an additional cpp file
//            to avoid unwanted changes in filter sequence when using the new
//            unified module framework code.
// $Log$
// Revision 1.15  2006/02/18 12:00:32  mellinger
// GetFilter<>() will now only return owned filters.
//
// Revision 1.14  2005/12/20 11:42:41  mellinger
// Added CVS id and log to comment.
//
//
// Description: This file declares a purely abstract GenericFilter interface
//   which all BCI2000 filters are supposed to implement.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef UGenericFilterH
#define UGenericFilterH

#include <typeinfo>
#include <set>
#include <list>
#include <map>
#include "UEnvironment.h"
// Includes needed for every filter, so they are put here for
// convenience.
#include "UGenericSignal.h"
#include "UBCIError.h"

#if( MODTYPE == 2 ) // Compiling for a Signal Processing module
# define RegisterFilter( name, pos )
# define Filter( name, pos )          _RegisterFilter( name, pos )
#else // MODTYPE
# define RegisterFilter( name, pos )  _RegisterFilter( name, pos )
#endif // MODTYPE

#define _RegisterFilter( name, pos )  GenericFilter::FilterRegistrar<name> name##Registrar(#pos);


class GenericFilter : protected Environment
{
 friend class Documentar;

 protected:
          GenericFilter()  { AllFilters().push_back( this ); }
 public:
  virtual ~GenericFilter() { AllFilters().remove( this ); }

  virtual void Preflight( const SignalProperties& Input,
                                SignalProperties& Output ) const = 0;
  // Initialize() performs initialization required when parameter settings
  // have changed.
  // Initialize2() will replace Initialize() in future versions.
  virtual void Initialize2( const SignalProperties& Input,
                            const SignalProperties& Output )    { Initialize(); }
  // The previous version of Initialize() -- will be removed in the future.
  virtual void Initialize() {}
  // StartRun() performs initialization required when starting a new run.
  virtual void StartRun()  { Initialize(); }
  // StopRun() performs actions required when a run ends.
  virtual void StopRun()   { Resting(); }

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
    { Registrars().insert( this ); }
    virtual ~Registrar()
    { Registrars().erase( this ); }
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
    static _registrarSet& Registrars();

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

  // Get available filters' position strings.
  static const std::string& GetFirstFilterPosition();
  static const std::string& GetLastFilterPosition();

  // Instantiate all registered filters once.
  static void InstantiateFilters();
  // Dispose of all filter instances not passed to another filter with PassFilter<>().
  static void DisposeFilters();
  // Apply the single filter functions to all filters instantiated and not passed
  // to another filter with PassFilter<>().
  static void PreflightFilters( const SignalProperties& Input,
                                      SignalProperties& Output );
  static void InitializeFilters();
  static void StartRunFilters();
  static void ProcessFilters( const GenericSignal* Input,
                                    GenericSignal* Output );
  static void StopRunFilters();
  static void RestingFilters();
  static void HaltFilters();

  // Create a new instance of the same type as the argument pointer.
  static GenericFilter* NewInstance( const GenericFilter* );
  // Get the first filter instance of a given type, e.g.:
  // MyFilter* myFilter = GenericFilter::GetFilter<MyFilter>();
  template<typename T> static T* GetFilter()
  {
    T* filterFound = NULL;
    filters_type::iterator i = OwnedFilters().begin();
    while( i != OwnedFilters().end() && filterFound == NULL )
    {
      filterFound = dynamic_cast<T*>( *i );
      ++i;
    }
    return filterFound;
  }
  // Get the first filter instance of a given type, and remove it
  // from the owned filters list. This is meant for building a hierarchy of
  // GenericFilter instances.
  template<typename T> static T* PassFilter()
  {
    T* filter = GetFilter<T>();
    OwnedFilters().remove( filter );
    return filter;
  }

 private:
  typedef std::list<GenericFilter*> filters_type;
  // This container holds all instantiated filters.
  static filters_type& AllFilters();
  // These are filters managed by the GenericFilter class:
  // Instantiation, Disposal, and application of filter functions.
  static filters_type& OwnedFilters();
  typedef std::map<GenericFilter*,GenericSignal> signals_type;
  static signals_type& OwnedSignals();
};

#endif // UGenericFilterH



