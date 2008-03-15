%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% $Id: addTopoMarker.m 2007-111-26 12:31:37EST fialkoff $ 
%% 
%% File: addTopoMarker.m 
%% 
%% Author: Joshua Fialkoff <fialkj@rpi.edu>, Gerwin Schalk <schalk@wadsworth.org>
%%
%% Description: This function adds an electrode marker to either a ECoG or 
%%  EEG topography plot
%%
%% (C) 2000-2008, BCI2000 Project
%% http:%%www.bci2000.org 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function [markerTextHandle] = addTopoMarker(x, y, label, params )
error(nargchk(4, 4,nargin));

  markerTextHandle = text(x,y,label,'HorizontalAlignment',params.markerTextHorzAlign,...
    'VerticalAlignment',params.markerTextVerAlign,'Color',params.markerTextColor,...
    'FontSize',params.markerTextFontSize, 'backgroundcolor', params.markerColor, ...
    'edgecolor', 'black');  
