function varargout=uvr2xyz(varargin)
%function [xyz]=uvr2xyz(uvr, center)

[X, Y, Z]=deal(1, 2, 3);
[U, V, R]=deal(1, 2, 3);

if nargin<3
   dims=[size(varargin{1}, 1) 1];
   uvr=varargin{1};
else
   dims=[size(varargin{R})];
   if all( ...
         sort(size(varargin{U}))==[1 dims(2)] & ...
         sort(size(varargin{V}))==[1 dims(1)] ...
         )
      [varargin{U} varargin{V}]=meshgrid(varargin{[U V]});
   end% if
   uvr=[varargin{U}(:) varargin{V}(:) varargin{R}(:)];
end% if
doTrasl=0;
if nargin==2 | nargin==4
   doTrasl=1;
   center=varargin{end};
end% if
if 0
   
   theta=atan(sqrt(tan(uvr(:, U)).^2+tan(uvr(:, V)).^2));
   %pXyz=[tan(uvr(:, [U V])) ones([size(uvr, 1) 1])];
   %pr=sqrt(sum(pXyz.^2, 2));
   %sXyz=pXyz./repmat(pr, [1 3]);
   %xyz=sXyz./repmat(uvr(:, R), [1 3]);
   pXy=tan(uvr(:, [U V]));
   pr=sqrt(sum(pXy.^2, 2)+1);
   coef=2*cos(theta)./pr;
   sXy=repmat(coef, [1 2]).*pXy;
   sZ=coef-1;
   xyz=[sXy sZ].*repmat(uvr(:, R), [1 3]);
else
   tU=tan(uvr(:, U));
   tV=tan(uvr(:, V));
   %coef=2./(1+tU.^2+tV.^2);
   coef=1./(1+tU.^2+tV.^2);
   x=uvr(:, R).*tU.*coef;
   y=uvr(:, R).*tV.*coef;
   %z=uvr(:, R).*coef-1;
   z=uvr(:, R).*coef;
   xyz=[x y z];
end% if
if doTrasl
   xyz=xyz+repmat(center(:)', [size(xyz, 1) 1]);
end% if

if nargout==1
   varargout{1}=[xyz];
elseif nargout==3
   [varargout{X:Z}]=deal(reshape(xyz(:, X), dims), reshape(xyz(:, Y), dims), reshape(xyz(:, Z), dims));
end% if
