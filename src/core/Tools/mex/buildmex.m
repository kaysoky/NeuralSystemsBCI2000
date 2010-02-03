function buildmex( varargin )
% Usage: buildmex <options> <target>
% For a list of options, see 'help mex'.
%
% Troubleshooting
%
% - The buildmex function fails with a 'Could not detect a compiler 
%   on local system' error.
% + Run 'mex -setup' to configure mex file compilation.
% 
% - Under Linux, compiling and linking works fine but I get an 
%   "Invalid MEX-file" error during the test stage, with the error
%   message indicating a failure to locate a required version of
%   GLIBCXX.
% + Matlab, and all software started from within it, use their own
%   copies of the GLIBC and GLIBCXX libraries, matching the gcc version
%   that was used to compile Matlab. When building a mex file, however,
%   the installed gcc will dynamically link with its own versions of those
%   libraries, which are sometimes incompatible with Matlab's (note that 
%   the list of officially supported gcc versions appears not to apply to 
%   MEX files using the C++ library, so you can't rely on your gcc version 
%   being listed there). Switching to the gcc version that your version of 
%   Matlab was built with should fix the problem.
%
% The buildmex function is part of the BCI2000 project.
% (C) 2000-2010, BCI2000 Project
% http://www.bci2000.org

%  Matlab M-file to build BCI2000 Matlab mex files.
%  $Id$

TARGETS = { ...
    'load_bcidat', ...
    'save_bcidat', ...
    'convert_bciprm', ...
    'mem', ...
    };

BCIROOT = '../../../../';
BCIFRM = [ BCIROOT 'src/shared/' ];
TESTFILE = 'testdata';
                    
MEXSRC = { ...
    [ BCIFRM 'fileio/dat/BCI2000FileReader.cpp' ], ...
    [ BCIFRM 'fileio/dat/BCI2000OutputFormat.cpp' ], ...
    [ BCIFRM 'fileio/edf_gdf/EDFOutputBase.cpp' ], ...
    [ BCIFRM 'fileio/edf_gdf/EDFOutputFormat.cpp' ], ...
    [ BCIFRM 'fileio/edf_gdf/GDFOutputFormat.cpp' ], ...
    [ BCIFRM 'fileio/edf_gdf/GDF.cpp' ], ...
    [ BCIFRM 'accessors/Environment.cpp' ], ...
    [ BCIFRM 'accessors/ParamRef.cpp' ], ...
    [ BCIFRM 'modules/MessageHandler.cpp' ], ...
    [ BCIFRM 'types/Param.cpp' ], ...
    [ BCIFRM 'types/ParamList.cpp' ], ...
    [ BCIFRM 'types/EncodedString.cpp' ], ...
    [ BCIFRM 'types/LabelIndex.cpp' ], ...
    [ BCIFRM 'types/HierarchicalLabel.cpp' ], ...
    [ BCIFRM 'types/Brackets.cpp' ], ...
    [ BCIFRM 'types/State.cpp' ], ...
    [ BCIFRM 'types/StateList.cpp' ], ...
    [ BCIFRM 'types/StateVector.cpp' ], ...
    [ BCIFRM 'types/StateVectorSample.cpp' ], ...
    [ BCIFRM 'types/GenericSignal.cpp' ], ...
    [ BCIFRM 'types/SignalProperties.cpp' ], ...
    [ BCIFRM 'types/SignalType.cpp' ], ...
    [ BCIFRM 'types/PhysicalUnit.cpp' ], ...
    [ BCIFRM 'types/SysCommand.cpp' ], ...
    [ BCIFRM 'types/Status.cpp' ], ...
    [ BCIFRM 'types/GenericVisualization.cpp' ], ...
    [ BCIFRM 'types/BitmapImage.cpp' ], ...
    [ BCIFRM 'utils/Expression/ArithmeticExpression.cpp' ], ...
    [ BCIFRM 'utils/Expression/Expression.cpp' ], ...
    [ BCIFRM 'utils/Expression/ExpressionParser.cpp' ], ...
    [ BCIFRM 'utils/ClassName.cpp' ], ...
    [ BCIFRM 'utils/MeasurementUnits.cpp' ], ...
    [ BCIFRM 'utils/VersionInfo.cpp' ], ...
    [ BCIFRM 'bcistream/BCIError.cpp' ], ...
    [ BCIFRM 'bcistream/BCIError_mex.cpp' ], ...
    'mexutils.cpp', ...
    };
          
INCLUDEPATHS = { ...
  '-I../..', ...
  [ '-I' BCIROOT '/src/extlib/math' ], ...
  [ '-I' BCIFRM ], ...
  [ '-I' BCIFRM '/accessors' ], ...
  [ '-I' BCIFRM '/bcistream' ], ...
  [ '-I' BCIFRM '/config' ], ...
  [ '-I' BCIFRM '/fileio' ], ...
  [ '-I' BCIFRM '/fileio/dat' ], ...
  [ '-I' BCIFRM '/fileio/edf_gdf' ], ...
  [ '-I' BCIFRM '/modules' ], ...
  [ '-I' BCIFRM '/types' ], ...
  [ '-I' BCIFRM '/utils' ], ...
  [ '-I' BCIFRM '/utils/Expression' ], ...
  };

BINDIR = [ BCIROOT '/tools/mex' ];

DEFINES = { ...
  '-DBCI_TOOL', ...
  '-DBCI_MEX', ...
  '-DNO_STRICT', ...
  '-D_NO_VCL', ...
  '-DNO_PCHINCLUDES', ...
  '-D_USE_MATH_DEFINES', ...
  };

switch( computer )
  case 'PCWIN'
    build_version_header = 'cmd /c "cd ..\..\..\shared\config && %ProgramFiles%\TortoiseSVN\bin\SubWCRev ..\.. Version.h.in Version.h"';
    CXXFLAGS = {};
    LDFLAGS = {};
  otherwise % we assume gcc on all other platforms
    build_version_header = '(cd ../../../buildutils && ./update_version_header.sh)';
    CXXFLAGS = { ...
      'CXXFLAGS="\$CXXFLAGS" -fPIC -include gccprefix.h' ...
      };
    LDFLAGS = { ...
      'LDFLAGS="\$LDFLAGS" -dead_strip' ...
      };
end;

options = {};
if( nargin < 1 )
  target = 'all';
else
  target = varargin{ nargin };
  options = { varargin{ 1 : end-1 } };
  if( target(1) == '-' )
    options = { options{:}, target };
    target = 'all';
  end
end

switch( target )

  case 'all'
    for( i = 1:length( TARGETS ) )
     buildmex( options{:}, TARGETS{i} );
    end
    buildmex test;
    
  case 'build'
    system( build_version_header );
    buildmex all;
    
  case 'test'
    fprintf( 1, [ 'Testing mex files ... ' ] );
    success = true;
    try
      [ signal, states, parameters ] = load_bcidat( [ TESTFILE '.dat' ] );
      ref = load( [ TESTFILE '.mat' ] );
      if( ~isempty( find( signal ~= ref.signal ) ) )
        error( 'Testing load_bcidat: Signal data mismatch' );
      end
      if( ~equal_structs( states, ref.states ) )
        error( 'Testing load_bcidat: State vector data mismatch' );
      end
      if( ~equal_structs( parameters, ref.parameters ) )
        error( 'Testing load_bcidat: Parameter mismatch' );
      end
      if( ~equal_structs( parameters, convert_bciprm( convert_bciprm( parameters ) ) ) )
        error( 'Testing convert_bciprm: Mismatch when converting forth and back' );
      end
      spectrum_ = mem( double( signal ), [16, 0, 0.4, 0.02, 15] );
      if( ~isempty( find( spectrum_ ~= ref.spectrum_ ) ) )
        rel_err = sqrt( norm( spectrum_ - ref.spectrum_, 'fro' ) / norm( spectrum_, 'fro' ) );
        fprintf( 1, 'Warning: mem: Mismatch between computed spectra (relative error is %d)\n', rel_err );
        err_limit = 1e-6;
        if( rel_err > err_limit )
          error( 'Testing mem: Mismatch between computed spectra exceeds relative error limit of %f\n', err_limit );
        end
      end
      clear signal states parameters spectrum ref;
    catch
      success = false;
    end
    if( success )
      fprintf( 1, 'Mex files tested OK.\n' );
    else
      err = lasterror;
      fprintf( 1, 'Error: %s.\n', err.message );
    end
    
  otherwise
    fprintf( 1, [ 'Building ' target ' ...\n' ] );
    mex( options{:}, CXXFLAGS{:}, LDFLAGS{:}, INCLUDEPATHS{:}, DEFINES{:}, [target '.cpp'], MEXSRC{:} );
    if( ~exist( BINDIR ) )
      mkdir( BINDIR );
    end
    copyfile( [ target '.' mexext ], BINDIR );
    if( exist( [ target '.m' ] ) )
      copyfile( [ target '.m' ], BINDIR );
    end
    
end

% A helper function to test structs for equality.
function result = equal_structs( inStruct1, inStruct2 )

  result = true;
  fnames = fieldnames( inStruct1 );
  if( length( fnames )~=length(fieldnames(inStruct2)) )
    result = false;
  elseif( ~strcmp( fnames, fieldnames( inStruct2 ) ) )
    result = false;
  else
    for( i = 1:length( fnames ) )
      if( isstruct( inStruct1.(fnames{i}) ) )
        if( ~equal_structs( inStruct1.(fnames{i}), inStruct2.(fnames{i}) ) )
          result = false;
        end
      elseif( ischar( inStruct1.(fnames{i}) ) || iscell( inStruct1.(fnames{i}) ) )
        if( ~strcmp( inStruct1.(fnames{i}), inStruct2.(fnames{i}) ) )
          result = false;
        end
      elseif( isnumeric( inStruct1.(fnames{i}) ) )
        result = isequalwithequalnans( inStruct1.(fnames{i}), inStruct2.(fnames{i}) );
      else
        if( ~isempty( find( inStruct1.(fnames{i}) ~= inStruct2.(fnames{i}) ) ) )
          result = false;
        end
      end
    end
  end
