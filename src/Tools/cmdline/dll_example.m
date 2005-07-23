function dll_example
% Example illustrating the usage of a BCI2000 filter dll from within
% MATLAB.
% Author: juergen.mellinger@uni-tuebingen.de
% Date:   Jul 20, 2005

filter_name = 'TransmissionFilter';
loadlibrary( filter_name, 'bci_dll', 'alias', 'lib' );
libfunctions( 'lib', '-full' );
try
  fprintf( 1, 'Filter Info:\n%s', calllib( 'lib', 'GetInfo' ) );
  % Add a state to the list of states.
  call_bcidll( 'lib', 'PutState', 'TestState 4 3 0 0' );
  % Instantiate the filter.
  call_bcidll( 'lib', 'Instantiate' );
  % Try reading and writing binary state vector information.
  % Note that binary state vectors from different DLLs are incompatible
  % unless they received identical state information with PutState()
  % before calling the DLL's Instantiate() function.
  statevectorLength = call_bcidll( 'lib', 'GetStatevectorLength', 0 );
  statevectorData = libpointer( 'uint8Ptr', uint8( zeros( statevectorLength, 1 ) ) );
  call_bcidll( 'lib', 'GetStatevector', statevectorData );
  statevector = get( statevectorData, 'Value' )
  call_bcidll( 'lib', 'SetStatevector', statevectorData );
  
  % Add some parameters to the parameter list.
% call_bcidll( 'lib', 'PutParameter', 'this is not a valid parameter line' );
  call_bcidll( 'lib', 'PutParameter', 'Source int SoftwareCh= 3' );
  call_bcidll( 'lib', 'PutParameter', 'Source int TransmitCh= 2' );
  call_bcidll( 'lib', 'PutParameter', 'Source intlist TransmitChList= 2 2 1' );

  % Call Preflight() to obtain signal output dimensions.
  inputSignal = [ 1 2 3 4; 5 6 7 8; 9 10 11 12 ]
  inputChannels = size( inputSignal, 1 );
  inputElements = size( inputSignal, 2 );
  [ outputChannels, outputElements ] = ...
    call_bcidll( 'lib', 'Preflight', inputChannels, inputElements );
  outputSignal = zeros( outputChannels, outputElements );
  % Initialize the DLL prior to calling Process().
  call_bcidll( 'lib', 'Initialize' );
  call_bcidll( 'lib', 'StartRun' );
  % Set a state value.
  call_bcidll( 'lib', 'SetStateValue', 'TestState', 6 );
  % Create pointers to input and output data.
  input = libpointer( 'doublePtr', inputSignal );
  output = libpointer( 'doublePtr', outputSignal );
  % Call Process() and read the output signal.
  call_bcidll( 'lib', 'Process', input, output );
  outputSignal = get( output, 'Value' )
  % Read a state value.
  [ stateName, value ] = call_bcidll( 'lib', 'GetStateValue', 'TestState', 0 )
  % Finish processing.
  call_bcidll( 'lib', 'StopRun' );
  % Reset the DLL's internal state.
  call_bcidll( 'lib', 'Dispose' );
catch
  fprintf( 1, '%s\n', lasterr );
end
unloadlibrary( 'lib' );

end

function varargout = call_bcidll( libname, funcname, varargin )
% Call a function from a BCI2000 filter DLL, and report errors if any.
result = cell( nargout + 1, 1 ); 
[ result{:} ] = calllib( libname, funcname, varargin{:} ); 
if( ~result{1} )
  error( 'Error calling %s:\n%s', funcname, calllib( libname, 'GetError' ) );
end
for( k = 1 : nargout )
  varargout{k} = result{k + 1};
end
end