clc;

DataVect = StatResults(7,:);
validChans = [1:59];

% [filename,pathname]=uigetfile;
% chansfullfilename=fullfile(pathname, filename);
chansfullfilename='Danilo050301_POINTS.txt';
fid=fopen(chansfullfilename,'r');
singleline=fgetl(fid);
while ~feof(fid)
    singleline=fgetl(fid);
    s = textscan(singleline, '%d %s %f %f %f');
    if ~isempty(find(validChans==s{1}))
        Lbls(s{1})= s{2};
        x(s{1}) = -s{3};
        y(s{1}) = -s{4};
        z(s{1}) = s{5};
    end
end

% -------------------XYZ------------------------------------
figure;
hold on;
pointHandle = plot3(x, y, z,'.');
textHandle = text(x, y, z,Lbls);
tri=delaunay(x,y);
trisurf(tri,x,y,z,DataVect);
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
axis equal;
view(2);

% -------------------UVR------------------------------------
figure;
hold on;
[u,v,r]=xyz2uvr(x,y,z);
pointHandle = plot3(u,v,r,'.');
textHandle = text(u,v,r,Lbls);
tri=delaunay(u,v);
trisurf(tri,u,v,r,DataVect);
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
axis equal;
view(2);


