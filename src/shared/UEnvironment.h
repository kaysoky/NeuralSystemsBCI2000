////////////////////////////////////////////////////////////////////////////////
//
// File: UEnvironment.h
//
// Description: EnvironmentBase and Environment are mix-in base classes that
//              channel access to enviroment-like
//              global objects of types CORECOMM, PARAMLIST, STATELIST,
//              STATEVECTOR, and provides convenient accessor functions
//              and checking utilities.
//              The difference between EnvironmentBase and Environment is that
//              Environment descendants are assumed to perform globally relevant
//              actions inside their constructors (as GenericFilter does), while
//              EnvironmentBase descendants such as Environment::Extension
//              are supposed to use a separate function Publish() for such
//              purposes. 
//
////////////////////////////////////////////////////////////////////////////////
#ifndef UEnvironmentH
#define UEnvironmentH

#include "UParameter.h"
#include "UState.h"
#include <set>
class std::ostream;
class SignalProperties;
class EnvironmentExtension;

// Some utility macros for better readable constructors.
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

// This base class channels access to Parameter, State, and Communication
// related objects that used to be arguments of member functions.
// "GenericFilter" inherits this class' "passive" accessor functions;
// to use its "controlling" functions, a class must be listed as
// a friend. Only framework classes should be friends.

class EnvironmentBase
{
 // Friends from framework classes.
 friend class GenericVisualization;
 friend class TfMain;

 friend class CoreModule;
 friend class Documentar;
 friend class StatusMessage;
 friend class FilterWrapper;

 // Protecting the constructor prevents instantiation of this class
 // outside its descendants.
 protected:
  EnvironmentBase() {}
  virtual ~EnvironmentBase() {}

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
    operator&();
   public:
    operator PARAMLIST*()   { return _paramlist; }
    PARAMLIST* operator->() { return _paramlist; }
  } Parameters;

  class statelistAccessor;
  friend class statelistAccessor;
  static class statelistAccessor
  {
   private:
    operator&();
   public:
    operator STATELIST*()   { return _statelist; }
    STATELIST* operator->() { return _statelist; }
  } States;

  class statevectorAccessor;
  friend class statevectorAccessor;
  static class statevectorAccessor
  {
   private:
    operator&();
   public:
    operator STATEVECTOR*()   { return _statevector; }
    STATEVECTOR* operator->() { return _statevector; }
  } Statevector;

 private:
  class operatorAccessor;
  friend class operatorAccessor;
  static class operatorAccessor
  {
   private:
    operator&();
   public:
    operator std::ostream*()   { return _operator; }
    std::ostream* operator->() { return _operator; }
  } Operator;

  // This macro defines some old identifiers that used to be arguments in
  // previous versions of GenericFilter's member functions.
#define TEMPORARY_ENVIRONMENT_GLUE \
  PARAMLIST* paramlist = (Environment::Parameters), *plist = paramlist; \
  STATELIST* statelist = (Environment::States), *slist = statelist; \
  STATEVECTOR* statevector = (Environment::Statevector), *svect = statevector; \

 // Convenient accessor functions. These are not static, so we can identify
 // the caller by its "this" pointer.
 protected:
  // Helper functions that include testing and reporting of error conditions.
  PARAM* GetParamPtr( const std::string& ) const;
  PARAM* GetOptionalParamPtr( const std::string& ) const;
  void CheckRange( const PARAM*, size_t, size_t ) const;

  // Read/write access to a parameter by its name and indices, if applicable.
  PARAM::type_adapter Parameter( const std::string& name,
                                 size_t index1 = 0,
                                 size_t index2 = 0 ) const;
  PARAM::type_adapter Parameter( const std::string& name,
                                 const std::string& label1,
                                 const std::string& label2 ) const;
  PARAM::type_adapter Parameter( const std::string& name,
                                 const std::string& label1,
                                 size_t index2 = 0 ) const;
  PARAM::type_adapter Parameter( const std::string& name,
                                 size_t index1,
                                 const std::string& label2 ) const;

 protected:
  // Read-only access to parameters that do not necessarily exist.
  const PARAM::type_adapter OptionalParameter( double defaultValue,
                                               const std::string& name,
                                               size_t index1 = 0,
                                               size_t index2 = 0 ) const;
  // A version with an empty default value, as often appropriate.
  const PARAM::type_adapter OptionalParameter( const std::string& name,
                                               size_t index1 = 0,
                                               size_t index2 = 0 ) const;

 private:
  const PARAM::type_adapter OptionalParameter(  const std::string& defaultValue,
                                                PARAM* param,
                                                size_t index1,
                                                size_t index2 ) const;
  const PARAM::type_adapter OptionalParameter(  double defaultValue,
                                                PARAM* param,
                                                size_t index1,
                                                size_t index2 ) const;
 protected:
  const PARAM::type_adapter OptionalParameter( double defaultValue,
                                               const std::string& name,
                                               const std::string& label1,
                                               const std::string& label2 ) const;
  const PARAM::type_adapter OptionalParameter( double defaultValue,
                                               const std::string& name,
                                               const std::string& label1,
                                               size_t index2 = 0 ) const;
  const PARAM::type_adapter OptionalParameter( double defaultValue,
                                               const std::string& name,
                                               size_t index1,
                                               const std::string& label2 ) const;

  // Versions with empty default values.
  const PARAM::type_adapter OptionalParameter( const std::string& name,
                                               const std::string& label1,
                                               const std::string& label2 ) const;
  const PARAM::type_adapter OptionalParameter( const std::string& name,
                                               const std::string& label1,
                                               size_t index2 = 0 ) const;
  const PARAM::type_adapter OptionalParameter( const std::string& name,
                                               size_t index1,
                                               const std::string& label2 ) const;

  // A macro/function combination for convenient formulation of parameter checks.
  #define PreflightCondition( x )        (_PreflightCondition(#x,double(x)))
  bool _PreflightCondition( const char*, bool ) const;

  // Read/write access a state by its name.
  STATEVECTOR::type_adapter State( const char* ) const;
  // Read-only access to states that are not required.
  // The first argument is a default value.
  const STATEVECTOR::type_adapter OptionalState( short, const char* ) const;
  const STATEVECTOR::type_adapter OptionalState( const char* name ) const
  { return OptionalState( 0, name ); }

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
  enum executionPhase
  {
    nonaccess,
    construction,
    preflight,
    initialization,
    processing,
    resting
  };

  static executionPhase GetPhase() { return _phase; }

 private:
  // Called to prevent access.
  static void EnterNonaccessPhase();
  // Called from the framework before any Environment descendant class
  // is instantiated.
  static void EnterConstructionPhase(   PARAMLIST*,
                                        STATELIST*,
                                        STATEVECTOR*,
                                        std::ostream* );
  // Called before any call to GenericFilter::Preflight().
  static void EnterPreflightPhase(      PARAMLIST*,
                                        STATELIST*,
                                        STATEVECTOR*,
                                        std::ostream* );
  // Called before any call to GenericFilter::Initialize().
  static void EnterInitializationPhase( PARAMLIST*,
                                        STATELIST*,
                                        STATEVECTOR*,
                                        std::ostream* );
  // Called before any call to GenericFilter::StartRun().
  static void EnterStartRunPhase(       PARAMLIST*,
                                        STATELIST*,
                                        STATEVECTOR*,
                                        std::ostream* );
  // Called before any call to GenericFilter::Process().
  static void EnterProcessingPhase(     PARAMLIST*,
                                        STATELIST*,
                                        STATEVECTOR*,
                                        std::ostream* );
  // Called before any call to GenericFilter::StopRun().
  static void EnterStopRunPhase(        PARAMLIST*,
                                        STATELIST*,
                                        STATEVECTOR*,
                                        std::ostream* );
  // Called before any call to GenericFilter::Resting().
  static void EnterRestingPhase(        PARAMLIST*,
                                        STATELIST*,
                                        STATEVECTOR*,
                                        std::ostream* );

 protected:
  void RegisterExtension( EnvironmentExtension* p )   { sExtensions.insert( p ); }
  void UnregisterExtension( EnvironmentExtension* p ) { sExtensions.erase( p ); }

 private:
  typedef std::set<EnvironmentExtension*> ExtensionsContainer;
  static ExtensionsContainer sExtensions;

 private:
  static PARAMLIST*     _paramlist;
  static STATELIST*     _statelist;
  static STATEVECTOR*   _statevector;
  static std::ostream*  _operator;
  static executionPhase _phase;
  // No direct use of those members, please.
  #define _paramlist    (void)
  #define _statelist    (void)
  #define _statevector  (void)
  #define _operator     (void)
  #define _phase        (void)
};

class Environment: protected EnvironmentBase
{
 // Protecting the constructor prevents instantiation of this class
 // outside its descendants.
 protected:
  Environment();
  virtual ~Environment() {}
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

class EnvironmentExtension : protected EnvironmentBase
{
  protected:
   EnvironmentExtension()          { EnvironmentBase::RegisterExtension( this ); }
   virtual ~EnvironmentExtension() { EnvironmentBase::UnregisterExtension( this ); }

  // The virtual interface.
  public:
   virtual void Publish() = 0;
   virtual void Preflight() const = 0;
   virtual void Initialize() = 0;
   virtual void StartRun() { Initialize(); }
   virtual void StopRun() { Resting(); }
   virtual void Process() = 0;
   virtual void Resting() {}
};

#endif // UEnvironmentH



