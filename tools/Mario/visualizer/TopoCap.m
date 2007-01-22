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
[u,v,r]=xyz2uvr(x,y,z);
tri=delaunay(u,v);
pointHandle = plot3(x, y, z,'.');
textHandle = text(x, y, z,Lbls);
trisurf(tri,x,y,z,DataVect);
shading interp;
MAXIMUM=max(max(abs(DataVect)));
caxis([-1 1]*MAXIMUM);
colorbar;
plot3(x,y,z,'k.');
axis equal;
view(2);
