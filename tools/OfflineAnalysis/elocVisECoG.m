%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% $Id: elocVisECoG.m 2007-111-26 12:31:37EST fialkoff $ 
%% 
%% File: elocVisECoG.m 
%% 
%% Author: Joshua Fialkoff <fialkj@rpi.edu>, Gerwin Schalk <schalk@wadsworth.org>
%%
%% Description: This function generates a visualization plot of an ECoG
%% electrode location file.
%%
%% (C) 2000-2008, BCI2000 Project
%% http:%%www.bci2000.org 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function [elecInfo] = elocVisECoG(params, loc_file)

error(nargchk(1,2,nargin));

% User Defined Defaults:
MAXCHANS = 256;

rmax = params.headRadius;

elecInfo = struct('elecNum', [], 'label', '', 'coords', [], 'markerHandle', [], 'markerTextHandle', []);

%%%%%%%%%%%%%%%%%%%%%%%
if nargin > 1
  fid = fopen(loc_file);
  if fid<1,
    error('Specified eloc file cannot be found');
    return;
  end
  A = fscanf(fid,'%d %f %f %s',[7 MAXCHANS]);
  fclose(fid);

  A = A';

  labels = setstr(A(:,4:7));
  idx = find(labels == '.');                       % some labels have dots
  labels(idx) = setstr(abs(' ')*ones(size(idx)));  % replace them with spaces

  elecNums = A(:,1);
  Th = pi/180*A(:,2);                              % convert degrees to radians
  Rd = A(:,3);
  ii = find(Rd <= 0.5);                     % interpolate on-head channels only
  Th = Th(ii);
  Rd = Rd(ii);
  labels = labels(ii,:);

  [y,x] = pol2cart(Th,Rd);      % transform from polar to cartesian coordinates
  
  for idx = 1:length(x)
    elecInfo(idx).elecNum = elecNums(idx);
    elecInfo(idx).coords = [x(idx) y(idx)];
    elecInfo(idx).label = labels(idx, :);
  end
  ha = gca;
  cla
  
end

hold on

% Draw grid
gridX([1 3]) = params.ecogExtremes(1:2);
gridX([2 4]) = params.ecogExtremes([2 1]);
gridY(1:2) = params.ecogExtremes(4);
gridY(3:4) = params.ecogExtremes(3);

fill(gridX, gridY, params.headFillColor);

% Draw grid outline
% plot(gridX, gridY,'color',params.headOutlineColor,'LineWidth',params.headOutlineWidth);
% plot(gridX(4:1), gridY(4:1),'color',params.headOutlineColor,'LineWidth',params.headOutlineWidth);

if nargin > 1
  % Plot Electrodes
  for idx = 1:size(labels,1)
    [markerHandle markerTextHandle] = addTopoMarker(x(idx), y(idx), int2str(elecInfo(idx).elecNum), params, 'ecog');
    elecInfo(idx).markerHandle = markerHandle;
    elecInfo(idx).markerTextHandle = markerTextHandle;
  end
end

hold off
axis tight
axis off

