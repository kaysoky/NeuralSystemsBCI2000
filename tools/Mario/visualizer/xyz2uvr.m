function varargout=xyz2uvr(varargin)
%function [uvr]=xyz2uvr(xyz, center)

[X, Y, Z]=deal(1, 2, 3);
[U, V, R]=deal(1, 2, 3);

if nargin<3
   dims=[size(varargin{1}, 1) 1];
   xyz=varargin{1};
else
   dims=[size(varargin{Z})];
   if size(varargin{X})==[dims(2) 1] & size(varargin{Y})==[dims(1) 1]
      [varargin{X} varargin{Y}]=meshgrid(varargin{[X Y]});
   end% if
   xyz=[varargin{X}(:) varargin{Y}(:) varargin{Z}(:)];
end% if
if nargin==2 | nargin==4
   center=varargin{end};
   xyz=xyz-repmat(center(:)', [size(xyz, 1) 1]);
end% if


r=sqrt(sum(xyz.^2, 2));
sXyz=xyz./repmat(r, [1 3]);
u=atan2(sXyz(:, X), sXyz(:, Z)+1);
v=atan2(sXyz(:, Y), sXyz(:, Z)+1);
if nargout==1
   varargout{1}=[u v r];
elseif nargout==3
   [varargout{U:R}]=deal(reshape(u, dims), reshape(v, dims), reshape(r, dims));
end% if
