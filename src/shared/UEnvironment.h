////////////////////////////////////////////////////////////////////////////////
//
// File: UEnvironment.h
//
// Description: A mix-in base class that channels access to enviroment-like
//              global objects of types CORECOMM, PARAMLIST, STATELIST,
//              STATEVECTOR, and provides convenient accessor functions
//              and checking utilities.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef UEnvironmentH
#define UEnvironmentH

//#include <typeinfo>
//#include <set>

#include "UParameter.h"
#include "UState.h"
class CORECOMM;
class SignalProperties;

// This base class channels access to Parameter, State, and Communication
// related objects that used to be arguments of member functions.
// "GenericFilter" inherits this class' "passive" accessor functions;
// to use its "controlling" functions, a class must be listed as
// a friend. Only framework classes should be friends.

class Environment
{
 // Friends from framework classes.
 friend class GenericVisualization;
 friend class TReceivingThread;
 friend class TfMain;
 friend class Documentar;
 friend class StatusMessage;
 friend class FILTERS;

 // Protecting the constructor prevents instantiation of this class
 // outside its descendants.
 protected:
  Environment();
  virtual ~Environment() {}

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

  class corecommAccessor;
  friend class corecommAccessor;
  static class corecommAccessor
  {
   private:
    operator&();
   public:
    operator CORECOMM*()   { return _corecomm; }
    CORECOMM* operator->() { return _corecomm; }
  } Corecomm;

  // This macro defines some old identifiers that used to be arguments in
  // previous versions of GenericFilter's member functions.
#define TEMPORARY_ENVIRONMENT_GLUE \
  PARAMLIST* paramlist = (Environment::Parameters), *plist = paramlist; \
  STATELIST* statelist = (Environment::States), *slist = statelist; \
  STATEVECTOR* statevector = (Environment::Statevector), *svect = statevector; \
  CORECOMM* corecomm = (Environment::Corecomm);

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
#ifdef LABEL_INDEXING
  PARAM::type_adapter Parameter( const std::string& name,
                                 const std::string& label1,
                                 const std::string& label2 ) const;
  PARAM::type_adapter Parameter( const std::string& name,
                                 const std::string& label1,
                                 size_t index2 = 0 ) const;
  PARAM::type_adapter Parameter( const std::string& name,
                                 size_t index1,
                                 const std::string& label2 ) const;
#endif // LABEL_INDEXING
  // Read-only access to parameters that need not be there.
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
#ifdef LABEL_INDEXING
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
#endif // LABEL_INDEXING

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

 private:
  enum executionPhase
  {
    nonaccess,
    construction,
    preflight,
    initialization,
    processing
  };

  // Called to prevent access.
  static void EnterNonaccessPhase();
  // Called from the framework before any Environment descendant class
  // is instantiated.
  static void EnterConstructionPhase(   PARAMLIST*,
                                        STATELIST*,
                                        STATEVECTOR*,
                                        CORECOMM* );
  // Called before any call to GenericFilter::Preflight().
  static void EnterPreflightPhase(      PARAMLIST*,
                                        STATELIST*,
                                        STATEVECTOR*,
                                        CORECOMM* );
  // Called before any call to GenericFilter::Initialize().
  static void EnterInitializationPhase( PARAMLIST*,
                                        STATELIST*,
                                        STATEVECTOR*,
                                        CORECOMM* );
  // Called before any call to GenericFilter::Process().
  static void EnterProcessingPhase(     PARAMLIST*,
                                        STATELIST*,
                                        STATEVECTOR*,
                                        CORECOMM* );

  static executionPhase GetPhase() { return _phase; }

 private:
  static PARAMLIST*     _paramlist;
  static STATELIST*     _statelist;
  static STATEVECTOR*   _statevector;
  static CORECOMM*      _corecomm;
  static executionPhase _phase;
  // No direct use of those members, please.
  #define _paramlist    (void)
  #define _statelist    (void)
  #define _statevector  (void)
  #define _corecomm     (void)
  #define _phase        (void)
};


#endif // UEnvironmentH



