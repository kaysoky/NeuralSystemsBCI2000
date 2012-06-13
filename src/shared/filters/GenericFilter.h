////////////////////////////////////////////////////////////////////////////////
// $Id$
// Description: This file declares a purely abstract GenericFilter interface
//   which all BCI2000 filters are supposed to implement.
//
// $BEGIN_BCI2000_LICENSE$
//
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2012: BCI2000 team and many external contributors ]
//
// BCI2000 is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// BCI2000 is distributed in the hope that it will be useful, but
//                         WITHOUT ANY WARRANTY
// - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <http://www.gnu.org/licenses/>.
//
// $END_BCI2000_LICENSE$
////////////////////////////////////////////////////////////////////////////////
#ifndef GENERIC_FILTER_H
#define GENERIC_FILTER_H

#include <typeinfo>
#include <set>
#include <list>
#include <map>
#include "Uncopyable.h"
#include "Environment.h"
#include "GenericVisualization.h"
// #includes needed for every filter, so they are put here for
// convenience.
#include "GenericSignal.h"
#include "BCIError.h"
#include "BCIRegistry.h"

class GenericFilter : protected Environment, private Uncopyable
{
 protected:
          GenericFilter();
 public:
  virtual ~GenericFilter();

 protected:
  // Traditionally, Parameters and States are announced from the Filter's
  // constructor. For greater flexibility, we are migrating towards a separate
  // Publish() method, but will maintain compatibility with existing code.
  virtual void Publish() {}
  virtual void Preflight( const SignalProperties& Input,
                                SignalProperties& Output ) const = 0;
  // Initialize() performs initialization required when parameter settings
  // have changed.
  virtual void Initialize( const SignalProperties& Input,
                           const SignalProperties& Output ) = 0;
  // StartRun() performs initialization required when starting a new run.
  virtual void StartRun()  {}
  // StopRun() performs actions required when a run ends.
  virtual void StopRun()   {}

  virtual void Process( const GenericSignal& Input,
                              GenericSignal& Output ) = 0;
  virtual void Resting() {}
  virtual void Halt() {}

  // This function is called once at instantiation, and after each Initialize().
  //  If it returns false at instantiation, no visualization parameter is created for the filter.
  //  If it returns false at initialization time, no default visualization is displayed,
  //  independently of the visualization parameter's value.
  //  (The filter itself may still maintain its own visualization object.)
  virtual bool AllowsVisualization() const { return true; }

 public: // Calling interface to virtual functions -- allows for setting up context.
  void CallPublish();
  void CallPreflight( const SignalProperties& Input,
                            SignalProperties& Output ) const;
  void CallInitialize( const SignalProperties& Input,
                       const SignalProperties& Output );
  void CallStartRun();
  void CallStopRun();
  void CallProcess( const GenericSignal& Input,
                          GenericSignal& Output );
  void CallResting();
  void CallHalt();

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
    Registrar( const char* inPos, int inPriority, bool inAutoDelete );
    virtual ~Registrar();
    const std::string& Position() const { return mPos; }
    virtual const std::type_info& Typeid() const = 0;
    virtual GenericFilter* NewInstance() const = 0;

   public:
    struct less
    {
      bool operator() ( const Registrar* a, const Registrar* b ) const;
    };
    friend struct less;

    typedef std::set<Registrar*, Registrar::less> RegistrarSet_;
    static RegistrarSet_& Registrars();

   private:
    std::string mPos;
    int mPriority;
    size_t mInstance;
    static size_t sCreatedInstances;

    struct AutoDeleteSet : public std::set<Registrar*>
    {
      ~AutoDeleteSet();
    };
    static AutoDeleteSet& AutoDeleteInstance();
  };
  typedef Registrar::RegistrarSet_ RegistrarSet;


 public:
  // Register a filter for automatic instantiation.
  template<typename T> class FilterRegistrar : public Registrar
  {
   public:
    FilterRegistrar( const char* inPos, int inPriority, bool inAutoDelete = false )
    : Registrar( inPos, inPriority, inAutoDelete )
    {}
    virtual const std::type_info& Typeid() const
    { return typeid( T ); }
    virtual GenericFilter* NewInstance() const
    { return new T; }
  };

  // Get available filters' position strings.
  static const std::string& GetFirstFilterPosition();
  static const std::string& GetLastFilterPosition();

  // Get info about the filter chain.
  struct ChainEntry
  { EncodedString position, name; };
  class ChainInfo : public std::vector<ChainEntry>
  {
   public:
    std::ostream& WriteToStream( std::ostream& );
    std::istream& ReadFromStream( std::istream& );
  };
  static ChainInfo GetChainInfo();

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
  static void ProcessFilters( const GenericSignal& Input,
                                    GenericSignal& Output );
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
    FiltersType::iterator i = OwnedFilters().begin();
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
  typedef std::list<GenericFilter*> FiltersType;
  // This container holds all instantiated filters.
  static FiltersType& AllFilters();
  // These are filters managed by the GenericFilter class:
  // Instantiation, Disposal, and application of filter functions.
  static FiltersType& OwnedFilters();
  typedef std::map<GenericFilter*,GenericSignal> SignalsType;
  static SignalsType& OwnedSignals();

  // Classes and functions related to default visualization.
  class FilterVis : public GenericVisualization
  {
   public:
    FilterVis()
         : mEnabled( false ) {}
    FilterVis( const FilterVis& f )
         : GenericVisualization( f ), mEnabled( f.mEnabled ) {}
    FilterVis& SetEnabled( bool b )
         { mEnabled = b; return *this; }
    bool Enabled() const
         { return mEnabled; }

   private:
    bool mEnabled;
  };
  typedef std::map<GenericFilter*, FilterVis> VisualizationsType;
  static VisualizationsType& Visualizations();
  std::string VisParamName() const;
};

#endif // GENERIC_FILTER_H



