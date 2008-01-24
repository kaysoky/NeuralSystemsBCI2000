%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% $Id: UGenericSignal.m 2007-111-26 12:31:37EST fialkoff $ 
%% 
%% File: UGenericSignal.m 
%% 
%% Author: Joshua Fialkoff <fialkj@rpi.edu>, Gerwin Schalk <schalk@wadsworth.org>
%%
%% Description: This function generates an electrode location file
%% compatible with topoplot and topoplotEEG.
%%
%% (C) 2000-2008, BCI2000 Project
%% http:%%www.bci2000.org 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function success = exportOfflineElocFile(elecData, fn, settings)
success = 0;

fid = fopen(fn, 'w+');
if fid == -1
  return;
end

for idxElec = 1:length(elecData)
  %0 degrees is towards the nose, so reverse coordinates in the function
  [theta rad] = cart2pol(elecData(idxElec).coords(2), ...
    elecData(idxElec).coords(1));
  theta = theta * 180/pi;
  %trim the label and check to make sure it has the right number of
  %characters
  label  = regexprep(elecData(idxElec).label, ...
    settings.strCleanUpRegexrep, '$1');
  if length(label) > settings.labelMaxLen
    label = label(1:settings.labelMaxLen);
  elseif length(label) < settings.labelMaxLen
    while length(label) < settings.labelMaxLen
      label(end+1) = settings.labelPadChar;
    end
  end
  fprintf(fid, '%d %.4f %.4f %s\r\n', idxElec, theta, rad, label);
end

fclose(fid);
success = 1;
return;