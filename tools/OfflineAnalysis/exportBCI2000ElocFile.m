%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% $Id: UGenericSignal.m 2007-111-26 12:31:37EST fialkoff $ 
%% 
%% File: UGenericSignal.m 
%% 
%% Author: Joshua Fialkoff <fialkj@rpi.edu>, Gerwin Schalk <schalk@wadsworth.org>
%%
%% Description: This function generates an electrode location file
%% compatible with BCI2000.
%%
%% (C) 2000-2008, BCI2000 Project
%% http:%%www.bci2000.org 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function success = exportBCI2000ElocFile(elecData, fn, settings)
success = 0;

fid = fopen(fn, 'w+');
if fid == -1
  return;
end

for idxElec = 1:length(elecData)
  %trim the label and check to make sure it has the right number of
  %characters
  label  = regexprep(elecData(idxElec).label, ...
    settings.strCleanUpRegexrep, '$1');
  fprintf(fid, '%s %.4f %.4f\r\n', label, elecData(idxElec).coords(1), ...
    elecData(idxElec).coords(2));
end

fclose(fid);
success = 1;
return;