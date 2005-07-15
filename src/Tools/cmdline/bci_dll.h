/*******************************************************************
// File:        bci_dll.h
// Date:        Jul 12, 2005
// Author:      juergen.mellinger@uni-tuebingen.de
// Description: Provides a basic structure for dynamically loaded
//              libraries that contain BCI2000 filters.
//
//              The interface has two levels:
//              A base level, where input and output are sent in
//              BCI2000 stream format, and a more convenient one
//              intended for use with, e.g., Matlab.
//
//              In the present preliminary implementation of the
//              convenience interface, data will be translated into
//              BCI2000 binary stream format before processing, and
//              results will be translated back from stream format
//              after processing.
//
//////////////////////////////////////////////////////////////////*/
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

/*********************** The low-level interface. *****************************/
typedef struct BCIDLL_bufferDesc
{
  char* data;
  long  length;
} BCIDLL_bufferDesc;

/*
function:  GetInfo
purpose:   Reports compilation and filter information.
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
function:  ProcessStreamData
purpose:   Reads messages in BCI2000 stream format from a buffer, and calls the filters
           contained in the DLL to process them.
arguments: Pointers to BCIDLL_bufferDesc structures containing/receiving output
           buffer and length.
           The output buffers themselves are allocated inside the DLL, and not meant to be
           deallocated by the caller.
           Any of the arguments may be NULL.
           A NULL input buffer argument calls the filters' StopRun() functions.
returns:   True if no error occurred.
*/
int DLLEXPORT
ProcessStreamData( BCIDLL_bufferDesc* inputBuffer,
                   BCIDLL_bufferDesc* outputBuffer,
                   BCIDLL_bufferDesc* visualizationBuffer,
                   BCIDLL_bufferDesc* errorBuffer );
/*
function:  FinishProcessing
purpose:   Analogue of suspending the on-line system. Results in StopRun()
           and Resting() being called, and StartRun() on the next call to
           ProcessStreamData().
           This can also be achieved by calling ProcessStreamData with a NULL argument
           for the input buffer.
arguments: Pointers receiving output buffer and length.
           The output buffers themselves are allocated inside the DLL, and not meant to be
           deallocated by the caller.
returns:   True if no error occurred.
*/

int DLLEXPORT
FinishProcessing( BCIDLL_bufferDesc* outputBuffer,
                  BCIDLL_bufferDesc* visualizationBuffer,
                  BCIDLL_bufferDesc* errorBuffer );

/********************* The convenience interface. *****************************/
/*
function:  ProcessState
purpose:   Processes a state given as a BCI2000 state definition line.
arguments: Pointer to a NULL terminated state line string.
returns:   True if no error occurred.
*/
int DLLEXPORT
ProcessState( char* stateLine );

/*
function:  ProcessParameter
purpose:   Processes a parameter given as a BCI2000 parameter line.
arguments: Pointer to a NULL terminated parameter line string.
returns:   True if no error occurred.
*/
int DLLEXPORT
ProcessParameter( char* parameterLine );

/*
function:  ProcessData
purpose:   Processes signal and state vector data.
arguments: Pointers holding signal buffer, signal dimensions,
           state vector data buffer, state vector length.
           On return, buffers will point to output data.
returns:   True if no error occurred.
*/
int DLLEXPORT
ProcessData( double** signal, long* channels, long* elements,
             unsigned char** statevector, long* statevectorlength );

/* Typedefs for function pointers. */
typedef char* __stdcall ( *GetInfoPtr )( void );
typedef char* __stdcall ( *GetErrorPtr )( void );

typedef int __stdcall ( *ProcessStreamDataPtr )
  ( BCIDLL_bufferDesc*, BCIDLL_bufferDesc*, BCIDLL_bufferDesc*, BCIDLL_bufferDesc* );
typedef int __stdcall ( *FinishProcessingPtr )
  ( BCIDLL_bufferDesc*, BCIDLL_bufferDesc*, BCIDLL_bufferDesc* );

typedef int __stdcall ( *ProcessStatePtr )( char* );
typedef int __stdcall ( *ProcessParameterPtr )( char* );
typedef int __stdcall ( *ProcessDataPtr )( double**, long*, long*, unsigned char**, long* );


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* BCI_DLL_H */
