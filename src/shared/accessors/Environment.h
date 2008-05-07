////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: Environment and EnvironmentExtension are mix-in base classes that
//              channel access to enviroment-like
//              global objects of types ParamList, StateList,
//              StateVector, and provides convenient accessor functions
//              and checking utilities.
//              The difference between Environment and EnvironmentExtension is that
//              Environment descendants are assumed to perform globally relevant
//              actions inside their constructors (as GenericFilter does), while
//              EnvironmentExtension descendants
//              are supposed to use a separate function Publish() for such
//              purposes.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "Param.h"
#include "ParamList.h"
#include "ParamRef.h"
#include "State.h"
#include "StateRef.h"
#include "BCIError.h"
#include "ClassName.h"
#include <set>
#include <iostream>
#include <sstream>

class SignalProperties;
class EnvironmentExtension;
class FilterWrapper;

// Some utility macros for better readable code in filter constructors.
#define BEGIN_PARAMETER_DEFINITIONS                                      \
{                                                                        \
  const char* params_[] =                                                \
  {

#define END_PARAMETER_DEFINITIONS                                        \
  };                                                                     \
  for( size_t i = 0; i < sizeof( params_ ) / sizeof( *params_ ); ++i )   \
  {                                                                      \
    Param p;                                                             \
    std::istringstream iss( params_[ i ] );                              \
    if( !( iss >> p ) )                                                  \
      bcierr << "error in parameter definition:\n"                       \
             << params_[ i ]                                             \
             << std::endl;                                               \
    else                                                                 \
    {                                                                    \
      p.Sections().push_back( bci::ClassName( typeid( *this ) ) );       \
      if( Parameters->Exists( p.Name() ) )                               \
        p.AssignValues( ( *Parameters )[ p.Name() ] );                   \
      Parameters->Add( p, -Instance() );                                 \
      bcidbg( 10 ) << "Registered parameter " << p.Name() << ", "        \
                   << "sorting by (" << -Instance() << ","               \
                   << p.Sections() << ")"                                \
                   << std::endl;                                         \
      OwnedParams()[ this ].insert( p.Name() );                          \
    }                                                                    \
  }                                                                      \
};

#define BEGIN_STATE_DEFINITIONS                                        \
{                                                                      \
  const char* states_[] =                                              \
  {

#define END_STATE_DEFINITIONS                                          \
  };                                                                   \
  for( size_t i = 0; i < sizeof( states_ ) / sizeof( *states_ ); ++i ) \
  {                                                                    \
    class State s;                                                     \
    std::istringstream iss( states_[ i ] );                            \
    if( !( iss >> s ) )                                                \
      bcierr << "error in state definition:\n"                         \
             << states_[ i ]                                           \
             << std::endl;                                             \
    else                                                               \
    {                                                                  \
      if( States->Exists( s.Name() ) )                                 \
        ( *States )[ s.Name() ].AssignValue( s );                      \
      else                                                             \
        States->Add( s );                                              \
      OwnedStates()[ this ].insert( s.Name() );                        \
    }                                                                  \
  }                                                                    \
};

// This base class channels access to Parameter, State, and Communication
// related objects that used to be arguments of member functions.
// "GenericFilter" inherits this class' "passive" accessor functions;
// to use its "controlling" functions, a class must be listed as
// a friend. Only framework classes should be friends.

class EnvironmentBase
{
 // Friends from framework classes.
 friend class GenericVisualization;

 friend class CoreModule;
 friend class StatusMessage;
 friend class FilterWrapper;

 // Protecting the constructor prevents instantiation of this class
 // outside its descendants.
 protected:
  EnvironmentBase()
    : mInstance( ++sNumInstances )
    {}
  virtual ~EnvironmentBase()
    { --sNumInstances; }
  int Instance() const
    { return mInstance; }

 // Opaque references to environment objects.
 // These symbols have the syntax of pointers but allow for intercepting
 // access to the underlying objects.
 // The typecast operators will be removed as soon as there is no more code
 // depending on true object pointers.
 protected:
  class paramlistAccessor;
  friend class paramlistAccessor;
  static class paramlistAccessor
  {
   private:
    paramlistAccessor* operator&();
   public:
    operator ParamList*()   { return paramlist_; }
    ParamList* operator->() { return paramlist_; }
  } Parameters;

  class statelistAccessor;
  friend class statelistAccessor;
  static class statelistAccessor
  {
   private:
    statelistAccessor* operator&();
   public:
    operator StateList*()   { return statelist_; }
    StateList* operator->() { return statelist_; }
  } States;

  class statevectorAccessor;
  friend class statevectorAccessor;
  static class statevectorAccessor
  {
   private:
    statevectorAccessor* operator&();
   public:
    operator StateVector*()   { return statevector_; }
    StateVector* operator->() { return statevector_; }
  } Statevector;

 private:
  class operatorAccessor;
  friend class operatorAccessor;
  static class operatorAccessor
  {
   private:
    operatorAccessor* operator&();
   public:
    operator std::ostream*()   { return operator_; }
    std::ostream* operator->() { return operator_; }
  } Operator;

 protected:
  // Helper function to construct and set an error context string.
  template<class T>
   static void ErrorContext( const std::string&, const T* );
  static void ErrorContext( const std::string& s );
  static const void* ObjectContext();

 private:
  static void ObjectContext( const void* );
  static const void* sObjectContext;

 // Convenient accessor functions. These are not static, so we can identify
 // the caller by its "this" pointer.
 protected:
  ParamRef Parameter( const std::string& name ) const;
  ParamRef OptionalParameter( const std::string& name,
                              const std::string& defaultValue = "" ) const;
  ParamRef OptionalParameter( const std::string& name,
                              double defaultValue ) const;
  static std::string DescribeValue( const Param&, size_t index1, size_t index2 );

 private:
  void ParamAccess( const std::string& name ) const;
  virtual void OnParamAccess( const std::string& name ) const {}

 protected:
  // A macro/function combination for convenient formulation of parameter checks.
  #define PreflightCondition( x )        (PreflightCondition_(#x,double(x)))
  bool PreflightCondition_( const char*, bool ) const;

  // Read/write access a state by its name.
  StateRef State( const std::string& name ) const;
  // Read-only access to states that are not required.
  // The second argument is a default value.
  const StateRef OptionalState( const std::string& name, short defaultValue = 0 ) const;

 private:
  void StateAccess( const std::string& name ) const;
  virtual void OnStateAccess( const std::string& name ) const {}

 // Controlling functions to be called from framework friends only.
 // In the future, these functions will be used to perform a number of
 // sanity checks / access control, e.g.
 // - Are these functions called in appropriate order?
 // - Are there any descendants instantiated outside the construction phase?
 // - Parameters may only be read/write accessed during the preflight and
 //   processing phases.
 // - Parameters accessed during the processing phase must have been accessed
 //   during the preflight phase, too -- this prevents a number of possible
 //   errors (typos in parameter names; newly introduced parameters not being
 //   checked during preflight; parameters being maintained in some class
 //   outside a GenericFilter).

 protected:
  enum ExecutionPhase
  {
    nonaccess,
    construction,
    preflight,
    initialization,
    startRun,
    processing,
    stopRun,
    resting
  };

  static ExecutionPhase Phase() { return phase_; }

 private:
  // Called to prevent access.
  static void EnterNonaccessPhase();
  // Called from the framework before any Environment descendant class
  // is instantiated.
  static void EnterConstructionPhase(   ParamList*,
                                        StateList*,
                                        StateVector*,
                                        std::ostream* );
  // Called before any call to GenericFilter::Preflight().
  static void EnterPreflightPhase(      ParamList*,
                                        StateList*,
                                        StateVector*,
                                        std::ostream* );
  // Called before any call to GenericFilter::Initialize().
  static void EnterInitializationPhase( ParamList*,
                                        StateList*,
                                        StateVector*,
                                        std::ostream* );
  // Called before any call to GenericFilter::StartRun().
  static void EnterStartRunPhase(       ParamList*,
                                        StateList*,
                                        StateVector*,
                                        std::ostream* );
  // Called before any call to GenericFilter::Process().
  static void EnterProcessingPhase(     ParamList*,
                                        StateList*,
                                        StateVector*,
                                        std::ostream* );
  // Called before any call to GenericFilter::StopRun().
  static void EnterStopRunPhase(        ParamList*,
                                        StateList*,
                                        StateVector*,
                                        std::ostream* );
  // Called before any call to GenericFilter::Resting().
  static void EnterRestingPhase(        ParamList*,
                                        StateList*,
                                        StateVector*,
                                        std::ostream* );

 protected:
  void RegisterExtension( EnvironmentExtension* p )   { Extensions().insert( p ); }
  void UnregisterExtension( EnvironmentExtension* p ) { Extensions().erase( p ); }

 private:
  typedef std::set<EnvironmentExtension*> ExtensionsContainer;
  static ExtensionsContainer& Extensions();

 protected:
  typedef std::set<std::string, Param::NameCmp> NameSet;
  static NameSet& ParamsRangeChecked();

  typedef std::map<const void*, NameSet> NameSetMap;
  static NameSetMap& OwnedParams();
  static NameSetMap& ParamsAccessedDuringPreflight();
  static NameSetMap& OwnedStates();
  static NameSetMap& StatesAccessedDuringPreflight();

 private:
  static ParamList*     paramlist_;
  static StateList*     statelist_;
  static StateVector*   statevector_;
  static std::ostream*  operator_;
  static ExecutionPhase phase_;
  // No direct use of those members, please.
  #define paramlist_    (void)
  #define statelist_    (void)
  #define statevector_  (void)
  #define operator_     (void)
  #define phase_        (void)

 private:
         int mInstance;
  static int sNumInstances;
};

// Environment adds consistency checks for Preflight() vs.
// later access to Parameters/States.
class Environment : protected EnvironmentBase
{
 protected:
  Environment() {}
  virtual ~Environment() {}

  using EnvironmentBase::OptionalParameter;

 private:
  virtual void OnParamAccess( const std::string& name ) const;
  virtual void OnStateAccess( const std::string& name ) const;
};

// A virtual interface for classes that provide global information and need
// to be notified of system phase transitions.
// Unlike GenericFilter descendants, descendants of EnvironmentExtension
// should not need any constructor code that publishes information; rather,
// they should use the Publish() virtual member function.
// That way we may have static instantiation without a separate registrar class,
// and avoid difficulties that arise from the fact that constructors are not
// ordinary member functions (constructors cannot be virtual; calling of virtual
// functions from base class constructors is impossible).

class EnvironmentExtension : protected Environment
{
  protected:
   EnvironmentExtension()          { Environment::RegisterExtension( this ); }
   virtual ~EnvironmentExtension() { Environment::UnregisterExtension( this ); }

  // The virtual interface.
  public:
   virtual void Publish() = 0;
   virtual void Preflight() const = 0;
   virtual void Initialize() = 0;
   virtual void PostInitialize() {}
   virtual void StartRun() {}
   virtual void PostStartRun() {}
   virtual void StopRun() {}
   virtual void PostStopRun() {}
   virtual void Process() {}
   virtual void PostProcess() {}
   virtual void Resting() {}
};

////////////////////////////////////////////////////////////////////////////////
// Template function definition
////////////////////////////////////////////////////////////////////////////////

// Helper function to construct and set an error context string.
template<class T>
void
EnvironmentBase::ErrorContext( const std::string& inQualifier, const T* inFilter )
{
  ObjectContext( inFilter );
  std::string context = bci::ClassName( typeid( *inFilter ) );
  context += "::";
  context += inQualifier;
  ErrorContext( context );
}

#endif // ENVIRONMENT_H




