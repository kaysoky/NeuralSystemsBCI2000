////////////////////////////////////////////////////////////////////////////////
// $Id$
//
// File: MatlabFilter.h
//
// Author: juergen.mellinger@uni-tuebingen.de
//
// Date: Nov 30, 2005
//
// $Log$
// Revision 1.1  2005/12/20 11:38:07  mellinger
// Initial version.
//
//
// Description: This BCI2000 filter calls the Matlab engine to act upon signals,
//    parameters, and states, thus providing the full BCI2000 filter interface
//    to a Matlab filter implementation.
//
//    For each BCI2000 filter member function, there is a corresponding Matlab
//    function as follows:
//
//    GenericFilter member      Matlab function syntax
//    ====================      ======================
//    Constructor               [parameters, states] = bci_Construct
//    Destructor                bci_Destruct
//    Preflight                 out_signal_dim = bci_Preflight( in_signal_dim )
//    Initialize                bci_Initialize( in_signal_dim, out_signal_dim )
//    Process                   out_signal = bci_Process( in_signal )
//    StartRun                  bci_StartRun
//    StopRun                   bci_StopRun
//    Resting                   bci_Resting
//    Halt                      bci_Halt
//
//    Existence of the above-listed Matlab functions is not mandatory.
//    The MatlabFilter uses the Matlab 'exist' command to determine whether a
//    given function is available, and will not call the Matlab engine when this
//    is not the case.
//    If either of the bci_Preflight, bci_Initialize, or bci_Process functions
//    is not available, a warning will be displayed to the user.
//
//    Parameters and states are accessible via global Matlab structs called
//    'bci_Parameters' and 'bci_States'. In Matlab, write
//
//      global bci_Parameters bci_States;
//      my_sampling_rate = bci_Parameters.SamplingRate;
//
//    Parameters may be changed from 'bci_StopRun' and 'bci_Resting', and will
//    automatically be propagated to the other modules.
//    State values may be modified from the 'bci_Process' function.
//
//    To add parameters and states to the BCI2000 list of states, the 'bci_Construct'
//    function may return non-empty cell arrays of strings in its 'parameters'
//    and 'states' return values. The strings constituting these cell arrays must
//    follow the BCI2000 parameter/state definition syntax as described in sections
//    3.2.4 and 3.2.5 of the "BCI2000 project outline" document.
//
//    BCI2000 signals are mapped to Matlab matrices with the channel index first,
//    and sample (element) index second.
//    Signal dimension arguments of bci_Preflight and bci_Initialize are
//    vectors of integers (1x2 matrices) as in '[n_channels n_elements]'.
//
//    To report errors from Matlab functions, use Matlab's error() command.
//
//    Troubleshooting:
//    If no Matlab instance opens up, execute
//      matlab /regserver
//    from the command line when logged in with administrative privileges.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef MatlabFilterH
#define MatlabFilterH

#include "UGenericFilter.h"
#include "UGenericVisualization.h"
#include "MatlabWrapper.h"

RegisterFilter( MatlabFilter, 2.C );

class MatlabFilter : public GenericFilter
{
 public:
          MatlabFilter();
  virtual ~MatlabFilter();
  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize2( const SignalProperties&, const SignalProperties& );
  virtual void Process( const GenericSignal* Input, GenericSignal* Output );
  virtual void StartRun();
  virtual void StopRun();
  virtual void Resting();
  virtual void Halt();

 private:
  void StatesToMatlabWS() const;
  void MatlabWSToStates();
  void ParamsToMatlabWS() const;
  void MatlabWSToParams();
  bool CallMatlab( MatlabFunction& ) const;
  
  MatlabFunction         mBci_Process;
  GenericVisualization   mVisualization;
  bool                   mVisualize;
};

#endif // MatlabFilterH


