function [rStr,mu,tri]=ResampMesh(struct,sph,center,translateFlag)

if nargin<3
    center=[0 0 0];
end

if nargin<4
    translateFlag=0;
else
    translateFlag=1;
end

DOTRI=0;
DOPOS=0;

if isstruct(sph)
    if isfield(sph,'elCoords')
        DOPOS=1;
        sphVert=[sph.refCoords;sph.elCoords];
    elseif isfield(sph,'vert')
        DOTRI=1;
        sphVert=sph.vert;
    else
        error('unknown structure')
    end
else
    sphVert=sph;
end;

centers=repmat(center,[size(sphVert,1) 1]);

if translateFlag
    centSph=sphVert+centers;
else
    centSph=sphVert;
end

[tri,mu]=reSampMeshC(struct.tri,struct.vert,[center;centSph]);
mu(tri==-1)=nan;

rStrVert=centers+(centSph-centers).*mu(:,[1 1 1]);

if DOTRI
    rStr=sph;
    rStr.vert=rStrVert;
elseif DOPOS
    rStr=sph;
    nRep=size(sph.refCoords,1);
    rStr.refCoords=rStrVert((1:nRep),:);
    rStr.elCoords=rStrVert((nRep+1:end),:);
else
    rStr=rStrVert;
end