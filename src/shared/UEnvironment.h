////////////////////////////////////////////////////////////////////////////////
// $Id$
//
// File: UEnvironment.h
//
// $Log$
// Revision 1.18  2006/01/17 17:39:44  mellinger
// Fixed list of project files.
//
// Revision 1.17  2006/01/11 19:08:44  mellinger
// Adaptation to latest revision of parameter and state related class interfaces.
//
// Revision 1.16  2005/12/20 11:42:41  mellinger
// Added CVS id and log to comment.
//
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
#include "ParamRef.h"
#include "UState.h"
#include "StateRef.h"
#include "UBCIError.h"
#include <set>
#include <iostream>
class SignalProperties;
class EnvironmentExtension;
class FilterWrapper;

// Some utility macros for better readable constructors.
#define BEGIN_PARAMETER_DEFINITIONS                                    \
{                                                                      \
  const char* _params[] =                                              \
  {

#define END_PARAMETER_DEFINITIONS                                      \
  };                                                                   \
  for( size_t i = 0; i < sizeof( _params ) / sizeof( *_params ); ++i ) \
    if( !Parameters->Add( _params[ i ] ) )               \
      bcierr << "error in parameter definition:\n"                     \
             << _params[ i ]                                           \
             << std::endl;                                             \
};

#define BEGIN_STATE_DEFINITIONS                                        \
{                                                                      \
  const char* _states[] =                                              \
  {

#define END_STATE_DEFINITIONS                                          \
  };                                                                   \
  for( size_t i = 0; i < sizeof( _states ) / sizeof( *_states ); ++i ) \
    if( !States->Add( _states[ i ] ) )                                 \
      bcierr << "error in state definition:\n"                         \
             << _states[ i ]                                           \
             << std::endl;                                             \
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
    paramlistAccessor* operator&();
   public:
    operator PARAMLIST*()   { return _paramlist; }
    PARAMLIST* operator->() { return _paramlist; }
  } Parameters;

  class statelistAccessor;
  friend class statelistAccessor;
  static class statelistAccessor
  {
   private:
    statelistAccessor* operator&();
   public:
    operator STATELIST*()   { return _statelist; }
    STATELIST* operator->() { return _statelist; }
  } States;

  class statevectorAccessor;
  friend class statevectorAccessor;
  static class statevectorAccessor
  {
   private:
    statevectorAccessor* operator&();
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
    operatorAccessor* operator&();
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
  PARAM* GetOptionalParamPtr( const std::string& inName,
                              const std::string& inLabel1,
                              const std::string& inLabel2,
                              int&               outIndex1,
                              int&               outIndex2 ) const;
  void CheckRange( const PARAM*, int, int ) const;

  // Read/write access to a parameter by its name and indices, if applicable.
  ParamRef Parameter( const std::string& name,
                      int   index1 = ParamRef::none,
                      int   index2 = ParamRef::none ) const;
  ParamRef Parameter( const std::string& name,
                      const std::string& label1,
                      const std::string& label2 ) const;
  ParamRef Parameter( const std::string& name,
                      const std::string& label1,
                      int   index2 = ParamRef::none ) const;
  ParamRef Parameter( const std::string& name,
                      int   index1,
                      const std::string& label2 ) const;

 protected:
  // Read-only access to parameters that do not necessarily exist.
  const ParamRef OptionalParameter( double defaultValue,
                                    const std::string& name,
                                    int index1 = ParamRef::none,
                                    int index2 = ParamRef::none ) const;
  const ParamRef OptionalParameter( double defaultValue,
                                    const std::string& name,
                                    const std::string& label1,
                                    const std::string& label2 ) const;
  const ParamRef OptionalParameter( double defaultValue,
                                    const std::string& name,
                                    const std::string& label1,
                                    int index2 = ParamRef::none ) const;
  const ParamRef OptionalParameter( double defaultValue,
                                    const std::string& name,
                                    int index1,
                                    const std::string& label2 ) const;

  // Versions with empty default values.
  const ParamRef OptionalParameter( const std::string& name,
                                    int index1 = ParamRef::none,
                                    int index2 = ParamRef::none ) const;
  const ParamRef OptionalParameter( const std::string& name,
                                    const std::string& label1,
                                    const std::string& label2 ) const;
  const ParamRef OptionalParameter( const std::string& name,
                                    const std::string& label1,
                                    int index2 = ParamRef::none ) const;
  const ParamRef OptionalParameter( const std::string& name,
                                    int index1,
                                    const std::string& label2 ) const;

 private:
  // Functions used by all OptionalParameter() functions.
  const ParamRef _OptionalParameter( const std::string& defaultValue,
                                     PARAM* param,
                                     int index1,
                                     int index2 ) const;
  const ParamRef _OptionalParameter( double defaultValue,
                                     PARAM* param,
                                     int index1,
                                     int index2 ) const;

 protected:
  // A macro/function combination for convenient formulation of parameter checks.
  #define PreflightCondition( x )        (_PreflightCondition(#x,double(x)))
  bool _PreflightCondition( const char*, bool ) const;

  // Read/write access a state by its name.
  StateRef State( const char* ) const;
  // Read-only access to states that are not required.
  // The first argument is a default value.
  const StateRef OptionalState( short, const char* ) const;
  const StateRef OptionalState( const char* name ) const
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
  void RegisterExtension( EnvironmentExtension* p )   { Extensions().insert( p ); }
  void UnregisterExtension( EnvironmentExtension* p ) { Extensions().erase( p ); }

 private:
  typedef std::set<EnvironmentExtension*> ExtensionsContainer;
  static ExtensionsContainer& Extensions();

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



