function mario(varargin)
% MARIO Startup the Mario environment
% TODO: write help lines
% 
%  Usage:   mario;
%           mario gui;
%           mario -gui;

% Authors:  Febo Cincotti, A. Emanuele Fiorilla, Marco Mattiocco, 
%           Juergen Mellinger, and Gerwin Shalk
%

% TODO:  revise copyright information in this and all other files and add
%        licence information

%  Copyright (C) 2007 Febo Cincotti, Fondazione Santa Lucia, Rome, Italy
%                     The BCI2000 Project, http://www.bci2000.org


%% Initialization
% Defaults
opt = struct( ...
   'LaunchGUI', true);

% Parse input parameters
argn = 0;
while (argn < nargin)
   argn = argn + 1;
   argument = varargin{argn};
   switch lower(argument)
      case {'gui' '-gui'}
         opt.LaunchGui = (argument(1) ~= '-');
      % case bla bla
         %
      otherwise
         error('Argument %s not recognized!', argument);
   end% switch
end% while

%% Set path 
% Determine install directory
mariofullfilename = which (mfilename);
mariopath = fileparts(mariofullfilename);

% recursively find subdirectories
excludepatterns = {'.svn' 'temp', 'bin', 'doc'};

mariopath = findsubfolders(mariopath, excludepatterns);

addpath(mariopath{:});

%% Launch
if opt.LaunchGUI
   mariogui;
end% if

%% ========================================================
function pathchunk = findsubfolders(rootpath, excludenames)

pathchunk = {rootpath};
tobeexcluded = [{'.' '..'} excludenames];
dd = dir(rootpath);
dirndx = find([dd(:).isdir]);
nitems = length(dirndx);
for ii = 1:nitems
   subdir = dd(dirndx(ii)).name;
   if strmatch(subdir, tobeexcluded)
      continue
   end% if
   subchunk = findsubfolders(fullfile(rootpath, subdir), excludenames);
   pathchunk = [pathchunk subchunk ];
end% for ii

