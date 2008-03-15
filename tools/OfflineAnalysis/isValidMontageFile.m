%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% $Id: UGenericSignal.m 2007-111-26 12:31:37EST fialkoff $ 
%% 
%% File: UGenericSignal.m 
%% 
%% Author: Joshua Fialkoff <fialkj@rpi.edu>, Gerwin Schalk <schalk@wadsworth.org>
%%
%% Description: This file checks the format of and values specified within
%% a given montage file in order to make sure the file is in the approved
%% format.
%%
%% (C) 2000-2008, BCI2000 Project
%% http:%%www.bci2000.org 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function [isValid numChans] = isValidMontageFile(fn, settings, acqType)
error(nargchk(3,3,nargin));

isValid = 1;
numChans = 0;

pat = '^\s*(?<idx>\d+)\s+(?<theta>-?\d+\.?\d*)\s+(?<r>\d+\.?\d*)\s+(?<lab>\S+)$';
fid=fopen(fn);
if fid == -1
  isValid = 0;
  return;
end
while 1
  tline = fgetl(fid);
  if ~ischar(tline)
      break;
  end

  [lineData tmp errorMsg] = sscanf(tline, '%d %f %f %s', 7);

  if size(lineData, 2) > 1 || ~isempty(errorMsg)
    isValid = 0;
    break;
  elseif isempty(errorMsg) && ~isempty(lineData) %skips blank lines
    numChans = numChans + 1;

    theta = lineData(2);
    r = lineData(3);

    if theta > 360 || theta < -360 || r < 0 || ...
        (strcmp(acqType, 'eeg') && r > settings.headRadius)
      isValid = 0;
      break;
    end      

  end
end
fclose(fid);

return;
