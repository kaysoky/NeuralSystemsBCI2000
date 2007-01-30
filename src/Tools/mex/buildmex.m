%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% $Id$
% File:        buildmex.m
% Date:        Jan 30, 2007
% Author:      juergen.mellinger@uni-tuebingen.de
% Description: Matlab M-file to build BCI2000 Matlab mex files.
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function buildmex( varargin )
% Usage: buildmex <options> <target>
% For a list of possible options, see 'help mex'.
%
% If the buildmex function fails with a 'Could not detect a compiler 
% on local system' error, run 'mex -setup'.

TARGETS = { ...
    'load_bcidat', ...
    'convert_bciprm', ...
    };

BCIROOT = '../../../';
BCIFRM = [ BCIROOT 'src/shared/' ];
CMDLINE = [ BCIROOT 'src/Tools/cmdline/' ];
                    
MEXSRC = { ...
    [ CMDLINE 'bci_stubs.cpp' ], ...
    [ BCIFRM 'UBCI2000Data.cpp' ], ...
    [ BCIFRM 'UParameter.cpp' ], ...
    [ BCIFRM 'EncodedString.cpp' ], ...
    [ BCIFRM 'UState.cpp' ], ...
    [ BCIFRM 'UGenericSignal.cpp'], ...
    [ BCIFRM 'UBCIError.cpp' ], ...
    'mexutils.cpp', ...
    };
          
INCLUDEPATHS = { ...
  '-I../..', ...
  [ '-I' BCIROOT ], ...
  [ '-I' BCIFRM ], ...
  };

BINDIR = '..\..\..\Tools\mex';

SUPPL = { 'load_bcidat.m', 'convert_bciprm.m' };

DEFINES = { ...
  '-D_DEBUG', ...
  '-DBCI_TOOL', ...
  '-DBCI_MEX', ...
  '-DNO_STRICT', ...
  '-D_NO_VCL', ...
  };

switch( computer( 'arch' ) )
  case 'win32'
    CXXFLAGS = {};
  otherwise % we assume gcc on all other platforms
    CXXFLAGS = { ...
      'CXXFLAGS=$CXXFLAGS -include gccprefix.h' ...
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
    
  otherwise
    mex( options{:}, CXXFLAGS{:}, INCLUDEPATHS{:}, DEFINES{:}, [target '.cpp'], MEXSRC{:} );

end
