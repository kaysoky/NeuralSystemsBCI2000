function out_signal = bci_Process( in_signal )

% Filter process demo
% 
% Apply a filter to in_signal, and return the result in out_signal.
% Signal dimensions are ( channels x samples ).

% BCI2000 filter interface for Matlab
% juergen.mellinger@uni-tuebingen.de, 2005
% (C) 2000-2007, BCI2000 Project
% http://www.bci2000.org

% Parameters and states are global variables.
global bci_Parameters bci_States;

% We use a global variable to store our filter's configuration.
global myFilterMatrix;

out_signal = myFilterMatrix * in_signal;

% For demonstration purposes, set the demo state:
bci_States.MyDemoState = 12 * rand;
