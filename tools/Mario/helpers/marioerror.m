function marioerror(errid, errmsg, varargin)
% MARIOERROR error handler for the Mario toolbox

STDERR =2;

try, argin = varargin; catch  argin ={}; end;

stack =dbstack;
errstr.identifier = sprintf('mario:%s:%s', stack(2).name, errid);
errstr.message = sprintf('Error using %s (function: %s) on line %d:\n%s', ...
   stack(2).file, stack(2).name, stack(2).line, sprintf(errmsg, varargin{:}));

% TODO:  define guierror vs. commandlineerror
%        logging
if 0
error(errstr);
else
   fprintf(STDERR, '[%s]\n???\t%s\n\n', errstr.identifier, errstr.message);
   beep;
end% if