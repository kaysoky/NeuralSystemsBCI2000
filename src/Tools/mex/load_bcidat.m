%LOAD_BCIDAT Load a BCI2000 data file into Matlab workspace variables.
%
%  [ signal, states, parameters ] = load_bcidat( 'filename' )
%
%  loads signal, state, and parameter data from the file whose name is given
%  in the function's argument.
%
%  The 'states' output variable will be a Matlab struct with BCI2000 state
%  names as struct member names, and the number of state value entries matching
%  the first dimension of the 'signal' output variable.
%
%  The 'parameters' output variable will be a Matlab struct with BCI2000
%  parameter names as struct member names.
%  Individual parameters are represented as cell arrays of strings, and may
%  be converted into numeric matrices by Matlab's str2double function.
%
%  The load_bcidat function is part of the BCI2000 project 
%  (http://www.bciresearch.org).

%  This is a help file documenting the functionality contained in
%  load_bcimat.mex.
%  $Id$
%  $Log$
%  Revision 1.1  2006/01/17 17:15:47  mellinger
%  Initial version.
%
