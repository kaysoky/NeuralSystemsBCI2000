%inizializzazione della figura
function hndls=DrawScalp(scalp,mapping,data)
% hndls=drawScalp(scalp, axHndl)

%base color
FLESHCOL=[.8 .7 .7];
%shrink factor for "graphic" scalp
SHRINKFACTOR = +0.1;

if nargin<3
    return
end

%fig init
figure
%getting axes handle
axHndl=axes;

%"data" scalp (will be limited to triangles covered by the cap)
patHndl=patch('Parent',axHndl,'faces',scalp.tri,'vertices',scalp.vert);
hold on
normals = get(patHndl,'VertexNormals');
nVers = normals./repmat(sqrt(sum(normals.^2, 2)), [1 3]);
%"graphic scalp" shrinked toward inside (will be limited to triangles not
%covered by the cap
patHndl2=patch('Parent',axHndl,'faces', scalp.tri,'vertices',scalp.vert + SHRINKFACTOR.*nVers);
hold off
%"graphic" scalp color vector
flCol=FLESHCOL(ones([size(scalp.vert, 1) 1]), :);
%"data" scalp vertices outside of the cap have trasparency setted to zero
%to make them invisible
%"data" scalp vertices inside of the cap have trasparency setted to one
%to make them visible; the vertices at the border of the cap have
%trasparency setted between one (inside the cap) and zero(outside the cap)
%according to the weight previously computed
mapAlpha = zeros([mapping.nVert 1]);
mapAlpha(mapping.validVert)=mapping.vertWeight;
%setting trasparency of "graphic" scalp vertices (using the opposite manner
%of the previous)
flAlpha = 1-mapAlpha;
%"data" scalp init color vector
col=zeros([mapping.nVert 1]);
col(mapping.validVert)=1;
%apply changes
set(patHndl,'FaceVertexCData',col,'FaceVertexAlphaData',mapAlpha,'FaceAlpha','interp','EdgeColor','none')
set(patHndl2,'FaceVertexCData', flCol,'FaceVertexAlphaData',flAlpha,'FaceAlpha','interp','EdgeColor','none')
material dull
shading interp

%setting lights
ligHndls=[
    light('position', [-1 0 .5], 'color', [1 1 1], 'Tag', 'frontLighth');
    light('position',[1 1 0], 'color', [.3 .3 .3], 'Tag', 'leftLighth');
    light('position',[1 -1 0], 'color', [.3 .3 .3], 'Tag', 'rightLighth');
    light('position',[0 0 1], 'color', [.5 .5 .5], 'Tag', 'topLighth');
    ];
lighting gouraud

%number of vertices
nVert=mapping.nVert;
%valid vertices
valid=mapping.validVert;
%interpolation matrix previously computed
map=mapping.mapMat;
%interpolated vector to plot on the scalp
dataVal=map*data;
%"data" scalp color vector
col=zeros([nVert 1]);
col(valid)=dataVal;
%apply changes
set(patHndl,'FaceVertexCData',col);

%setting cbar limits modalities
switch mapping.cLimMode
    case 'auto'
        mapping.cLim=[min(dataVal) max(dataVal)];
    case 'symauto'
        mapping.cLim=[-1 1]*max(abs(dataVal));
    case 'zeroauto'
        [absMax ndxMax]=max(abs(dataVal));
        if dataVal(ndxMax)>0
            mapping.cLim=[0 1]*absMax;
        else
            mapping.cLim=[-1 0]*absMax;
        end
    case '+zeroauto'
        mapping.cLim=[0 1]*max(0, max(dataVal));
    case '-zeroauto'
        mapping.cLim=[1 0]*min(0, min(dataVal));
    otherwise
        warning
end
%apply cbar limits
caxis(mapping.cLim);

%getting handles
figHndl=get(axHndl,'Parent');
set(figHndl,'Renderer', 'opengl');
set(axHndl,'DataAspectRatio',[1 1 1],'PlotBoxAspectRatio', [1 1 1]);
hndls.figHndl=figHndl;
hndls.axHndl=axHndl;
hndls.patHndl=patHndl;
hndls.patHndl2=patHndl2;
hndls.ligHndls=ligHndls;