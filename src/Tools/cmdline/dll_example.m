function dll_example

% Example to illustrate the usage of a BCI2000 filter dll from within
% MATLAB.

filter_name = 'TransmissionFilter';
loadlibrary( filter_name, 'bci_dll', 'alias', 'lib' );
libfunctions( 'lib', '-full' );

try
  
  fprintf( 1, 'Filter Info:\n%s', calllib( 'lib', 'GetInfo' ) );

  if( ~calllib( 'lib', 'Instantiate' ) )
    fprintf( 1, 'Errors in Instantiate:\n%s\n', calllib( 'lib', 'GetError' ) );
  end;

  if( ~calllib( 'lib', 'PutParameter', 'this is not a valid parameter line' ) )
    fprintf( 1, 'Errors in PutParameter:\n%s\n', calllib( 'lib', 'GetError' ) );
  end;

  if( ~calllib( 'lib', 'PutParameter', 'Source int SoftwareCh= 3' ) )
    fprintf( 1, 'Errors in PutParameter:\n%s\n', calllib( 'lib', 'GetError' ) );
  end;

  if( ~calllib( 'lib', 'PutParameter', 'Source int TransmitCh= 2' ) )
    fprintf( 1, 'Errors in PutParameter:\n%s\n', calllib( 'lib', 'GetError' ) );
  end;

  if( ~calllib( 'lib', 'PutParameter', 'Source intlist TransmitChList= 2 2 1' ) )
    fprintf( 1, 'Errors in PutParameter:\n%s\n', calllib( 'lib', 'GetError' ) );
  end;

  [ success, outputChannels, outputElements ] = calllib( 'lib', 'Preflight', 3, 4 );
  if( ~success )
    fprintf( 1, 'Errors in Preflight:\n%s\n', calllib( 'lib', 'GetError' ) );
    error( 'Aborting.' );
  end;

  if( ~calllib( 'lib', 'Initialize' ) )
    fprintf( 1, 'Errors in Initialize:\n%s\n', calllib( 'lib', 'GetError' ) );
    error( 'Aborting.' );
  end;

  inputSignal = libpointer( 'doublePtr', [ 1 2 3 4; 5 6 7 8; 9 10 11 12 ] );
  outputSignal = libpointer( 'doublePtr', zeros( outputChannels, outputElements ) );
  if( ~calllib( 'lib', 'Process', inputSignal, outputSignal ) )
    fprintf( 1, 'Errors in Process:\n%s\n', calllib( 'lib', 'GetError' ) );
    error( 'Aborting.' );
  end;

  inputSignal = get( inputSignal, 'Value' )
  outputSignal = get( outputSignal, 'Value' )
  
  if( ~calllib( 'lib', 'Dispose' ) )
    fprintf( 1, 'Errors in Dispose:\n%s\n', calllib( 'lib', 'GetError' ) );
  end;
  
catch

  fprintf( 1, '%s\n', 'A call to a DLL function resulted in an error.' );

end

unloadlibrary( 'lib' );
