function bci_Initialize( in_signal_dims, out_signal_dims )

% Filter initialize demo
% 
% Perform configuration for the bci_Process script.

% BCI2000 filter interface for Matlab
% juergen.mellinger@uni-tuebingen.de, 2005

% Parameters and states are global variables.
global bci_Parameters bci_States;

% We use a global variable for our filter configuration as well.
global myFilterMatrix;
myFilterMatrix = str2double( bci_Parameters.MyFilterMatrix );
