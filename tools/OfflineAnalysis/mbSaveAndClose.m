%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% $Id: msgbox2.m 2007-03-14 12:31:37EST fialkoff $ 
%% 
%% File: mbSaveAndClose.m 
%% 
%% Author: Joshua Fialkoff <fialkj@rpi.edu>, Gerwin Schalk <schalk@wadsworth.org>
%%
%% Description: This function is a helper function for msgbox2 that enables
%% writing to the settings.mat state file.
%%
%% (C) 2000-2008, BCI2000 Project
%% http:%%www.bci2000.org 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function mbSaveAndClose(fig)
%save settings
settings = load('settings.mat');
settings = settings.settings;
globalParams = get(fig, 'userdata');
settings = setfield(settings, globalParams.paramName, get(globalParams.check, 'value')== 0); 
save settings.mat settings;
delete(fig);

