function bci_Initialize( in_signal_dims, out_signal_dims )

% Filter initialize demo
% 
% Perform configuration for the bci_Process script.

% BCI2000 filter interface for Matlab
% juergen.mellinger@uni-tuebingen.de, 2005
% (C) 2000-2008, BCI2000 Project
% http://www.bci2000.org

% Parameters and states are global variables.
global bci_Parameters bci_States;

% We use global variables for our filter configuration as well.
global myFilterOffsets;
myFilterOffsets = str2double( bci_Parameters.MyFilterOffsets );
global myFilterMatrix;
myFilterMatrix = str2double( bci_Parameters.MyFilterMatrix );
