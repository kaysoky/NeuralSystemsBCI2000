function varargout = topo3(BCI2k,x,y)
DataVect = BCI2k.Statistics.Data(x,:)';
validChans = find(BCI2k.Conditioning.Geom.MMF.ValidChannels);
Lbls = BCI2k.Conditioning.Geom.MMF.ElectrodeLabels(validChans);
if isfield (BCI2k.Conditioning.Geom.MMF, 'SpatialPositions')
   x = Montage.SpatialPositions(validChans, 1);
   y = Montage.SpatialPositions(validChans, 2);
   z = Montage.SpatialPositions(validChans, 3);
   uvr = xyz2uvr([-x -y z]);
   u = uvr(:, 1);
   v = uvr(:, 2);
   r = uvr(:, 3);
else
   Grid = BCI2k.Conditioning.Geom.MMF.SpatialGrid;
   u=zeros([length(validChans) 1]);
   v=u; r=u;
   [nGridRows nGridCols] = size(Grid);
   uAxis = linspace(-pi/4, pi/4, nGridRows);
   vAxis = linspace(-pi/4, pi/4, nGridCols);
   for rr = 1:nGridRows
      for cc = 1:nGridCols
         CurChan = Grid(rr, cc);
         if CurChan == 0, continue, end;
         validNdx = find(CurChan==validChans);
         if isempty(validNdx), continue, end;
         u(CurChan) = uAxis(rr);
         v(CurChan) = vAxis(cc);
         r(CurChan) = 100;
      end% for cc
   end% for rr
   xyz = uvr2xyz([u v r]);
   x = xyz(:, 1);
   y = xyz(:, 2);
   z = xyz(:, 3);
end% if

% ===========================
% Interpolate
iuAxis = linspace(min(u), max(u), 50);
ivAxis = linspace(min(v), max(v), 50);

[iu_grid iv_grid] = meshgrid(iuAxis, ivAxis);
ir_grid    = griddata(u, v, r,iu_grid,iv_grid, 'cubic');
iData_grid = griddata(u, v, double(DataVect),iu_grid,iv_grid, 'cubic');
%
iu = iu_grid(:);
iv = iv_grid(:);
ir = ir_grid(:);
iuvr = [iu iv ir];
[ix iy iz] = uvr2xyz(iu, iv, ir);
ixyz = [ix iy iz];
iData = iData_grid(:);
% -------------------XYZ (3D)------------------------------------
figure(sum('mario3dnointerp'));
clf;
hold on;
textHandle = text(x, y, z,Lbls);
tri=delaunay(u,v);
itri=delaunay(iu,iv);
% trisurf(tri,x,y,z,DataVect);
trisurf(itri,ix,iy,iz,iData);
shading interp;
MAXIMUM=max(max(abs(DataVect)));
caxis([-1 1]*MAXIMUM);
colorbar;
plot3(x,y,z,'k.');
% patch ellisse rosa
fi = linspace(0,2*pi,100);
ax = (max(x)-min(x))/2;
ay = (max(y)-min(y))/2;
x1 = ax*cos(fi);
y1 = ay*sin(fi);
z1 = 0*x1;
% z1 = -1 + 0*x1;
patch(x1,y1,z1,[1 .7 .7])
hold off;
axis equal off;
set(gcf, 'Renderer', 'opengl')
rotate3d on
view(3);

% -------------------UVR (2D) ------------------------------------
figure(sum('mariouvnointerp'));
clf;
hold on;
pointHandle = plot3(u,v,r,'.');
textHandle = text(u,v,r,Lbls);
tri=delaunay(u,v);
itri=delaunay(iu,iv);
% trisurf(tri,u,v,r,DataVect);
trisurf(itri,iu,iv,ir,iData);
hold off
shading interp;
MAXIMUM=max(max(abs(DataVect)));
caxis([-1 1]*MAXIMUM);
colorbar;
% % % % % % patch ellisse rosa
% % % % % fi = linspace(0,2*pi,100);
% % % % % ax = (max(x)-min(x))/2;
% % % % % ay = (max(y)-min(y))/2;
% % % % % x1 = ax*cos(fi);
% % % % % y1 = ay*sin(fi);
% % % % % z1 = 0*x1;
% % % % % [u1,v1,r1]=xyz2uvr(x1,y1,z1);
% % % % % patch(u1,v1,z1,[1 .7 .7])
axis equal off ;
view(2);


if nargout
   varargout{1} = gcf
end% if