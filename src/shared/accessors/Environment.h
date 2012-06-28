////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: Environment is a mix-in base class
//   that channels access to enviroment-like
//   global objects of types ParamList, StateList,
//   StateVector, and provides convenient accessor functions which perform
//   checks for parameter/state existence, and bounds.
//
//   Inheriting from Environment, EnvironmentExtension is an interface class
//   for components that need to handle system-wide events such as Preflight,
//   Initialize, etc. Direct inheritance from Environment should be used
//   for components that need access to parameters and states without handling
//   global events.
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
#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "Param.h"
#include "ParamList.h"
#include "ParamRef.h"
#include "State.h"
#include "StateList.h"
#include "StateRef.h"
#include "BCIError.h"
#include "BCIException.h"
#include "ClassName.h"
#include "OSThreadLocal.h"
#include "BCIRegistry.h"
#include <set>
#include <iostream>
#include <sstream>

#define IS_SRC_MODULE ( MODTYPE == 1 )

class SignalProperties;
class EnvironmentExtension;

class CoreModule;
class FilterWrapper;

class ApplicationWindowClient;

// Some utility macros for better readable code in filter constructors.
#define BEGIN_PARAMETER_DEFINITIONS                                      \
{                                                                        \
  const char* params_[] =                                                \
  {

#define END_PARAMETER_DEFINITIONS                                        \
  };                                                                     \
  ::EncodedString className_( bci::ClassName( typeid( *this ) ) );       \
  std::ostringstream oss_;                                               \
  className_.WriteToStream( oss_, ":" );                                 \
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
      p.Sections().push_back( oss_.str() );                              \
      if( Parameters->Exists( p.Name() ) )                               \
        p.AssignValues( ( *Parameters )[ p.Name() ] );                   \
      Parameters->Add( p, -Instance() );                                 \
      bcidbg( 10 ) << "Registered parameter " << p.Name() << ", "        \
                   << "sorting by (" << -Instance() << ","               \
                   << p.Sections() << ")"                                \
                   << std::endl;                                         \
      OwnedParams()[ObjectContext()].insert( p.Name() );                 \
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
      s.SetKind( State::StateKind );                                   \
      if( States->Exists( s.Name() ) )                                 \
      {                                                                \
        if( ( *States )[ s.Name() ].Kind() == State::EventKind )       \
          bcierr << "trying to define state "                          \
                 << s.Name()                                           \
                 << ", has been previously defined as an event"        \
                 << std::endl;                                         \
        else                                                           \
          ( *States )[ s.Name() ].AssignValue( s );                    \
      }                                                                \
      else                                                             \
        States->Add( s );                                              \
      OwnedStates()[ObjectContext()].insert( s.Name() );               \
    }                                                                  \
  }                                                                    \
};

#define BEGIN_EVENT_DEFINITIONS                                        \
{                                                                      \
  const char* events_[] =                                              \
  {

#if IS_SRC_MODULE

#define END_EVENT_DEFINITIONS                                          \
  };                                                                   \
  for( size_t i = 0; i < sizeof( events_ ) / sizeof( *events_ ); ++i ) \
  {                                                                    \
    class State s;                                                     \
    std::istringstream iss( events_[ i ] );                            \
    if( !( iss >> s ) )                                                \
      bcierr << "error in state definition:\n"                         \
             << events_[ i ]                                           \
             << std::endl;                                             \
    else                                                               \
    {                                                                  \
      s.SetKind( State::EventKind );                                   \
      if( States->Exists( s.Name() ) )                                 \
      {                                                                \
        if( ( *States )[ s.Name() ].Kind() == State::StateKind )       \
          bcierr << "trying to define event "                          \
                 << s.Name()                                           \
                 << ", has been previously defined as a state"         \
                 << std::endl;                                         \
        else                                                           \
          ( *States )[ s.Name() ].AssignValue( s );                    \
      }                                                                \
      else                                                             \
        States->Add( s );                                              \
      OwnedStates()[ObjectContext()].insert( s.Name() );               \
    }                                                                  \
  }                                                                    \
};

#else // IS_SRC_MODULE

#define END_EVENT_DEFINITIONS                                           \
  };                                                                    \
  bcierr << "Trying to define events outside a source module." << endl; \
};

#endif // IS_SRC_MODULE

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
  EnvironmentBase();
  EnvironmentBase( ParamList&, StateList&, StateVector& );
  virtual ~EnvironmentBase();
  int Instance() const
    { return mInstance; }

 private:
 // Opaque references to environment objects.
 // These symbols have the syntax of pointers but allow for intercepting
 // access to the underlying objects.
  template<class T> class Accessor_
  {
   friend class EnvironmentBase;
   private:
    Accessor_* operator&();
    explicit Accessor_( T* inLocal )
    : mpLocal( NULL )
    { // The environment of an EnvironmentBase object is
      // determined
      // 1) by a constructor argument,
      // 2) by the object context currently active in the thread,
      // 3) by a "wrapper context" currently active in the thread.
      // This mechanism ensures that EnvironmentBase objects that
      // are part of, or created by, GenericFilter or
      // EnvironmentExtension objects "inherit" their environment
      // from their "parent" objects.
      // "Wrapper contexts" may be set during instantiation of
      // EnvironmentBase objects to attach created objects to
      // a non-global environment.
      if( inLocal )
        mpLocal = inLocal;
      else if( stObjectContext )
        mpLocal = stObjectContext->Get_<T>();
      else if( stWrapperContext )
        mpLocal = stWrapperContext->Get_<T>();
    }
   public:
    operator T*() const
    { return mpLocal ? mpLocal : spGlobal; }
    T* operator->() const
    { return operator T*(); }
   private:
    T* mpLocal;
    static T* spGlobal;
  };
  friend class Accessor_<ParamList>;
  friend class Accessor_<StateList>;
  friend class Accessor_<StateVector>;

 protected:
  Accessor_<ParamList> Parameters;
  Accessor_<StateList> States;
  Accessor_<StateVector> Statevector;

 private:
  template<class T> const Accessor_<T>& Get_() const;

 protected:
  // Instantiate a RAII WrapperContext object to temporarily set a wrapper context.
  // WrapperContexts are necessary because EnvironmentBase descendants may access
  // Parameters and States from their constructors where no ObjectContext is available.
  class WrapperContext
  {
   public:
    WrapperContext( const EnvironmentBase* p )
    : mPrevious( stWrapperContext )
    { stWrapperContext = p; }
    ~WrapperContext()
    { stWrapperContext = mPrevious; }
   private:
    const EnvironmentBase* mPrevious;
  };
  friend class WrapperContext;

 private:
  bool IsGlobalEnvironment() const;

 protected:
  // Helper functions to construct and set an error context string.
  static void ErrorContext( const std::string&, const EnvironmentBase* = NULL );
  const EnvironmentBase* ObjectContext() const;

  // There is a bug in the Borland compiler that prevents casting the this
  // pointer of a subclass that inherits "protected"ly from EnvironmentBase into
  // an EnvironmentBase*. We fix this by providing an explicit conversion for
  // that case.
  const EnvironmentBase* Base() const { return this; }

 private:
  static OSThreadLocal<const EnvironmentBase*> stObjectContext,
                                               stWrapperContext;

 // Convenient accessor functions. These are not static, so we can identify
 // the caller as an object.
 protected:
  // The Parameter()/OptionalParameter() functions allow access to parameters by name.
  ParamRef Parameter( const std::string& name ) const;
  ParamRef OptionalParameter( const std::string& name,
                              const std::string& defaultValue = "" ) const;
  ParamRef OptionalParameter( const std::string& name,
                              double defaultValue ) const;
  static std::string DescribeValue( const Param&, size_t index1, size_t index2 );

 private:
  void ParamAccess( const std::string& name ) const;
  virtual void OnParamAccess( const std::string& name ) const {}

  mutable Param mDefaultParam;

 protected:
  // A macro/function combination for convenient formulation of parameter checks.
  #define PreflightCondition( x )        (PreflightCondition_(#x,double(x)))
  bool PreflightCondition_( const char*, bool ) const;
  // Read/write access a state by its name.
  StateRef State( const std::string& name ) const;
  // Read-only access to states that are not required.
  // The second argument is a default value.
  StateRef OptionalState( const std::string& name, State::ValueType defaultValue = 0 ) const;

 private:
  const StateList* StateListAccess() const;
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
                                        StateVector* );
  // Called before any call to GenericFilter::Preflight().
  static void EnterPreflightPhase(      ParamList*,
                                        StateList*,
                                        StateVector* );
  // Called before any call to GenericFilter::Initialize().
  static void EnterInitializationPhase( ParamList*,
                                        StateList*,
                                        StateVector* );
  // Called before any call to GenericFilter::StartRun().
  static void EnterStartRunPhase(       ParamList*,
                                        StateList*,
                                        StateVector* );
  // Called before any call to GenericFilter::Process().
  static void EnterProcessingPhase(     ParamList*,
                                        StateList*,
                                        StateVector* );
  // Called before any call to GenericFilter::StopRun().
  static void EnterStopRunPhase(        ParamList*,
                                        StateList*,
                                        StateVector* );
  // Called before any call to GenericFilter::Resting().
  static void EnterRestingPhase(        ParamList*,
                                        StateList*,
                                        StateVector* );

 protected:
  void RegisterExtension__( EnvironmentExtension* p ) { Extensions().insert( p ); }
  void UnregisterExtension( EnvironmentExtension* p ) { Extensions().erase( p ); }

 private:
  typedef std::set<EnvironmentExtension*> ExtensionsContainer;
  static ExtensionsContainer& Extensions();
  static ExtensionsContainer& ExtensionsPublished();

 protected:
  typedef std::set<std::string, Param::NameCmp> NameSet;
  static NameSet& ParamsRangeChecked();

  typedef std::map<const EnvironmentBase*, NameSet> NameSetMap;
  static NameSetMap& OwnedParams();
  static NameSetMap& ParamsAccessedDuringPreflight();
  static NameSetMap& OwnedStates();
  static NameSetMap& StatesAccessedDuringPreflight();

 private:
  static ExecutionPhase phase_;
  // No direct use of the phase_ member, please.
  #define phase_        (void)

 private:
  int mInstance;
  static int sMaxInstanceID;
};

// Environment adds consistency checks for Preflight() vs.
// later access to Parameters/States.
class Environment : public EnvironmentBase
{
  // Friends from framework classes.
  friend class CoreModule;
  friend class FilterWrapper;
  friend class ApplicationWindowClient;

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
  EnvironmentExtension()          { Environment::RegisterExtension__( this ); }
  virtual ~EnvironmentExtension() { Environment::UnregisterExtension( this ); }

 // The extension interface. Extension descendants implement these functions.
 // For an overview of the events handled by these functions, see the documentation
 // of the GenericFilter function.
 protected:
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

 // The public calling interface to virtual functions.
 // Each Call...() function sets up an error context, and calls its corresponding
 // virtual function.
 public:
  void CallPublish();
  void CallPreflight() const;
  void CallInitialize();
  void CallPostInitialize();
  void CallStartRun();
  void CallPostStartRun();
  void CallStopRun();
  void CallPostStopRun();
  void CallProcess();
  void CallPostProcess();
  void CallResting();

 public:
  // Objects registered with AutoDelete() are deleted at static deinitialization time
  static EnvironmentExtension* AutoDelete( EnvironmentExtension* p );

 private:
  struct AutoDeleteSet : public std::set<EnvironmentExtension*>
  {
    ~AutoDeleteSet() { while( !empty() ) { delete *begin(); erase( begin() ); } }
  };
  static AutoDeleteSet& AutoDeleteInstance();
  friend struct AutoDeleteSet;
};

template<>
inline
const EnvironmentBase::Accessor_<ParamList>&
EnvironmentBase::Get_<ParamList>() const
{
  return Parameters;
}

template<>
inline
const EnvironmentBase::Accessor_<StateList>&
EnvironmentBase::Get_<StateList>() const
{
  return States;
}

template<>
inline
const EnvironmentBase::Accessor_<StateVector>&
EnvironmentBase::Get_<StateVector>() const
{
  return Statevector;
}

#endif // ENVIRONMENT_H





