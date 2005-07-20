/*********************************************************************/
/* File:        bci_dll.h
/* Date:        Jul 12, 2005
/* Author:      juergen.mellinger@uni-tuebingen.de
/* Description: Provides a framework for dlls that contain BCI2000
/*              filters.
/*
/*********************************************************************/
#ifndef BCI_DLL_H
#define BCI_DLL_H

#ifdef BCI_DLL
# define DLLEXPORT __stdcall __declspec( dllexport )
#else
# define DLLEXPORT __stdcall
#endif // BCI_DLL

#ifdef __cplusplus
extern "C" {
#pragma }
#endif /* __cplusplus */

/*
function:  GetInfo
purpose:   Reports filter name and compilation information.
arguments: None.
returns:   Pointer to a null-terminated string holding the information requested.
           The output buffer is allocated inside the DLL, and not meant to be
           deallocated by the caller.
*/
char* DLLEXPORT
GetInfo( void );

/*
function:  GetError
purpose:   Retrieve error output from the previously called function.
arguments: None.
returns:   Pointer to a null-terminated string containing error output.
           The output buffer is allocated inside the DLL, and not meant to be
           deallocated by the caller.
*/
char* DLLEXPORT
GetError( void );

/*
function:  PutParameter
purpose:   Parses a BCI2000 parameter definition line, and adds the resulting
           parameter object to the filter's parameter list, or changes the value of
           a parameter if it exists.
arguments: Pointer to a NULL terminated parameter line string.
returns:   True (1) if no error occurred.
*/
int DLLEXPORT
PutParameter( char* parameterLine );

/*
function:  PutState
purpose:   Parses a BCI2000 state definition line, and adds the resulting state
           object to the filter's state list.
arguments: Pointer to a NULL terminated state line string.
returns:   True (1) if no error occurred.
*/
int DLLEXPORT
PutState( char* stateLine );

/*
function:  SetState
purpose:   Sets the value of a state to a given value.
arguments: Pointer to a NULL terminated state name string; new state value.
returns:   True (1) if no error occurred.
*/
int DLLEXPORT
SetStateValue( char* stateName, short value );

/*
function:  GetState
purpose:   Gets the value of a state.
arguments: Pointer to a NULL terminated state name string; pointer to state value.
returns:   True (1) if no error occurred.
*/
int DLLEXPORT
GetStateValue( char* stateName, short* valuePtr );

/*
function:  SetStatevector
purpose:   Sets the DLL's state vector to the binary values contained in a state vector.
arguments: Pointer and length of state vector data. The length must match the length of the
           state vector inside the DLL.
returns:   True (1) if no error occurred.
*/
int DLLEXPORT
SetStatevector( char* statevectorData, long statevectorLength );

/*
function:  Instantiate
purpose:   Instantiate the filters contained in the dll.
arguments: n/a
returns:   True (1) if no error occurred.
*/
int DLLEXPORT
Instantiate( void );

/*
function:  Dispose
purpose:   Dispose of all filter instances.
arguments: n/a
returns:   True (1) if no error occurred.
*/
int DLLEXPORT
Dispose( void );

/*
function:  Preflight
purpose:   Calls the filters' Preflight() function.
arguments: Pointers to input signal dimensions.
returns:   True (1) if no error occurred;
           dimensions pointers contain output signal dimensions on return.
*/
int DLLEXPORT
Preflight( long* ioChannels, long* ioElements );

/*
function:  Initialize
purpose:   Calls the filters' Initialize() function.
arguments: n/a
returns:   True (1) if no error occurred.
*/
int DLLEXPORT
Initialize( void );

/*
function:  StartRun
purpose:   Calls the filters' StartRun() function.
arguments: n/a
returns:   True (1) if no error occurred.
*/
int DLLEXPORT
StartRun( void );

/*
function:  Process
purpose:   Calls the filters' Process() function.
arguments: Pointer to input array pointer.
           Input and output arrays are expected to have the dimensions specified/obtained
           using the Preflight() function.
returns:   True (1) if no error occurred.
*/
int DLLEXPORT
Process( double* inputSignal, double* outputSignal );

/*
function:  StopRun
purpose:   Calls the filters' StopRun() function.
arguments: n/a
returns:   True (1) if no error occurred.
*/
int DLLEXPORT
StopRun( void );

/*
function:  Resting
purpose:   Calls the filters' Resting() function.
arguments: n/a
returns:   True (1) if no error occurred.
*/
int DLLEXPORT
Resting( void );

/*
function:  Halt
purpose:   Calls the filters' Halt() function.
arguments: n/a
returns:   True (1) if no error occurred.
*/
int DLLEXPORT
Halt( void );

/* Typedefs for function pointers intended to simplify calling GetProcAddress(). */
typedef char* __stdcall ( *GetInfoPtr )( void );
typedef char* __stdcall ( *GetErrorPtr )( void );
typedef int __stdcall ( *PutParameterPtr )( char* );
typedef int __stdcall ( *PutStatePtr )( char* );
typedef int __stdcall ( *SetStateValuePtr )( char*, long );
typedef int __stdcall ( *GetStateValuePtr )( char*, long* );
typedef int __stdcall ( *SetStatevectorPtr )( char*, long );
typedef int __stdcall ( *InstantiatePtr )( void );
typedef int __stdcall ( *DisposePtr )( void );
typedef int __stdcall ( *PreflightPtr )( long*, long* );
typedef int __stdcall ( *InitializePtr )( void );
typedef int __stdcall ( *StartRunPtr )( void );
typedef int __stdcall ( *ProcessPtr )( double*, double* );
typedef int __stdcall ( *StopRunPtr )( void );
typedef int __stdcall ( *RestingPtr )( void );
typedef int __stdcall ( *HaltPtr )( void );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* BCI_DLL_H */
