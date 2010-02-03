function [ out_signal_dim ] = bci_Preflight( in_signal_dim )

% Filter preflight demo
% 
% Check whether parameters and states are accessible, and whether
% parameters have values that allow for safe processing by the 
% bci_Process function.
% Report any errors as demonstrated below.
% Also, report output signal dimensions in the 'out_signal_dim' argument.

% BCI2000 filter interface for Matlab
% juergen.mellinger@uni-tuebingen.de, 2005
% (C) 2000-2010, BCI2000 Project
% http://www.bci2000.org

% Parameters and states are global variables.
global bci_Parameters bci_States;

out_signal_dim = in_signal_dim;
out_signal_dim( 1 ) = size( bci_Parameters.MyFilterMatrix, 1 );
if( in_signal_dim( 1 ) ~= size( bci_Parameters.MyFilterMatrix, 2 ) )
  error( [ ...
    'MyFilterMatrix'' number of columns must match the input' ...
    ' signal''s number of channels' ...
  ] );
end
if( size( bci_Parameters.MyFilterMatrix, 1 ) ~= size( bci_Parameters.MyFilterOffsets, 1 ) )
  error( [ ...
    'MyFilterOffsets'' number of entries must match MyFilterMatrix'' ' ...
    ' number of rows' ...
  ] );
end
% Check whether this parameter is available.
bci_Parameters.MyRunCount;

% Check whether the state is available.
bci_States.MyDemoState; 

% Parameters are cell arrays of strings.
% str2double will convert cell arrays of strings into numerical matrices.
if( str2double( bci_Parameters.MyReportBogusError ) )
  error( 'Bogus error for demo purposes' );
end

