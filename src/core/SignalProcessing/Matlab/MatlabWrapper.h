////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: Wrapper classes for convenient creation and manipulation of
//              Matlab workspace variables, and calling of Matlab functions.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef MatlabWrapperH
#define MatlabWrapperH

#include <string>
#include <vector>
#include "GenericSignal.h"
#include "Param.h"
#include "Engine.h"

class MatlabEngine
{
 public:
  class DoubleMatrix : public std::vector<std::vector<double> >
  {
   public:
    DoubleMatrix() {}
    DoubleMatrix( double d )
    : std::vector<std::vector<double> >( 1, std::vector<double>( 1, d ) ) {}

    DoubleMatrix( const GenericSignal& );
    DoubleMatrix( const SignalProperties& );
    operator GenericSignal() const;
    operator SignalProperties() const;
  };

  class StringMatrix : public std::vector<std::vector<std::string> >
  {
   public:
    StringMatrix() {}
    StringMatrix( const std::string& s )
    : std::vector<std::vector<std::string> >( 1, std::vector<std::string>( 1, s ) ) {}

    StringMatrix( const Param& );
    operator Param() const;
  };

 protected:
  MatlabEngine();
  virtual ~MatlabEngine();

 public:
  static bool         CreateGlobal( const std::string& name );
  static bool         ClearVariable( const std::string& name );

  static std::string  GetString( const std::string& expr );
  static bool         PutString( const std::string& expr, const std::string& value );

  static double       GetScalar( const std::string& expr );
  static bool         PutScalar( const std::string& expr, double value );

  static DoubleMatrix GetMatrix( const std::string& expr );
  static bool         PutMatrix( const std::string& expr, const DoubleMatrix& value );


  static StringMatrix GetCells(  const std::string& expr );
  static bool         PutCells(  const std::string& expr, const StringMatrix& value );

 private:
  static mxArray* GetMxArray( const std::string& expr );
  static bool     PutMxArray( const std::string& expr, const mxArray* value );

 protected:
  static int      sNumInstances;
  static Engine*  spEngineRef;

  typedef struct { void** mProc; const char* mName; } ProcNameEntry;
  static bool  LoadDLL( const char* name, int numProcs, ProcNameEntry* );
  static const std::string& OSError( long );

  // Matlab Engine DLL imports
  static const char* sLibEngName;
  static Engine*     ( *engOpen )( const char* );
  static int         ( *engClose )( Engine* );
  static int         ( *engEvalString )( Engine*, const char* );
  static mxArray*    ( *engGetVariable )( Engine*, const char* );
  static int         ( *engPutVariable )( Engine*, const char*, const mxArray* );
  static ProcNameEntry sEngProcNames[];

  // Matlab MX DLL imports
  static const char* sLibMxName;
  static mxArray*    ( *mxCreateString )( const char* );
  static char*       ( *mxArrayToString )( const mxArray* );
  static mxArray*    ( *mxCreateCellMatrix )( int, int );
  static mxArray*    ( *mxGetCell )( const mxArray*, int );
  static void        ( *mxSetCell )( mxArray*, int, mxArray* );

  static mxArray*    ( *mxCreateNumericMatrix )( int, int, mxClassID, int );
  static double*     ( *mxGetPr )( const mxArray* );
  static void        ( *mxSetPr )( mxArray*, double* );

  static int         ( *mxGetNumberOfDimensions )( const mxArray* );
  static const int*  ( *mxGetDimensions )( const mxArray* );
  static int         ( *mxCalcSingleSubscript )( const mxArray*, int, const int* );

  static void        ( *mxDestroyArray )( mxArray* );
  static void        ( *mxFree )( void* );
  static ProcNameEntry sMxProcNames[];
};

class MatlabFunction : private MatlabEngine
{
 public:
  MatlabFunction( const std::string& );
  ~MatlabFunction();

 private:
  MatlabFunction( const MatlabFunction& );

 public:
  MatlabFunction& InputArgument( const std::string& );
  MatlabFunction& OutputArgument( const std::string& );
  const std::string& Name() const { return mName; }
  bool Exists() const             { return mExists; }
  const std::string& Execute() const;

 private:
  std::vector<std::string> mInputArguments,
                           mOutputArguments;
  std::string mName;
  bool mExists;
};

#endif // MatlabWrapperH


