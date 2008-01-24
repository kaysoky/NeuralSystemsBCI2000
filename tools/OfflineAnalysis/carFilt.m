%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% $Id: carFilt.m 2007-111-26 12:31:37EST fialkoff $ 
%% 
%% File: carFilt.m 
%% 
%% Author: Joshua Fialkoff <fialkj@rpi.edu>, Gerwin Schalk <schalk@wadsworth.org>
%%
%% Description: This function performs CAR filtering on the specified
%% signal.
%%
%% (C) 2000-2008, BCI2000 Project
%% http:%%www.bci2000.org 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function [signalOut] = carFilt(signal, chanDim)

num_chans = size(signal, chanDim);

spatfiltmatrix=[];
% create a CAR spatial filter
spatfiltmatrix=-ones(num_chans);
for i=1:num_chans
 spatfiltmatrix(i, i)=num_chans-1;
end

signalOut=double(signal)*spatfiltmatrix;