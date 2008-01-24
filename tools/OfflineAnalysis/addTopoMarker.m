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
function [markerHandle markerTextHandle] = addTopoMarker(x, y, label, params,  mode)
error(nargchk(5, 5,nargin));

if nargin >=5 && strcmpi(mode, 'ecog')
    markerWidth = params.markerWidthECoG;
    markerHeight = params.markerHeightECoG;
  else
    markerWidth = params.markerWidth;
    markerHeight = params.markerHeight;
  end

  markerHandle = ...
    fill([x-markerWidth/2 x+markerWidth/2 x+markerWidth/2 x-markerWidth/2], ...
      [y+markerHeight/2 y+markerHeight/2 y-markerHeight/2 y-markerHeight/2], params.markerColor);

  markerTextHandle = text(x,y,label,'HorizontalAlignment',params.markerTextHorzAlign,...
    'VerticalAlignment',params.markerTextVerAlign,'Color',params.markerTextColor,...
    'FontSize',params.markerTextFontSize);  