%% Function Definition
function [] = DoMapOnScalp (scalp,posFullFileName,DataVect)


%% Flag & Constants
%Enable Scalp Downsampling
SurfaceDwnSmpl=false;
%Enable Colorbar view
CBAR=false;
%Enable Montage
ADDmnt=true;
%Enable Texture Mapping
TEXTUREmapping=false;
%Interpolation Parameters
mode='sphinterp';
lambda=0.013;
%Spatial Reference
CENTER=[0 0 50];


%% Getting Data, Electrodes Position & Surface
%Getting Data Vector
if nargin<3
    [DataFileName DataFilePath]=uigetfile('*.mat','SELECT DATA VECTOR');
    DataFullFileName=fullfile(DataFilePath,DataFileName);
    load(DataFullFileName,'DataVect');
    clear DataFilePath DataFileName DataFullFileName
end;
%Getting Electrode Positions
if nargin<2
    [posFileName,posFilePath]=uigetfile({'*_ATT.dig';'*.pos'},'SELECT ELECTRODE POSITIONS FILE');
    posFullFileName=fullfile(posFilePath,posFileName);
    clear posFilePath posFileName
end;
%Getting Surface
if nargin<1
    [scalpFileName,scalpFilePath]=uigetfile('*.mat','SELECT SCalp');
    scalpFullFileName=fullfile(scalpFilePath,scalpFileName);
    file=load(scalpFullFileName,'-mat');
    clear scalpFileName scalpFilePath scalpFullFileName
    if isfield(file,'scalp')
        scalp=file.scalp;
    elseif isfield(file,'head')
        scalp=file.head;
    elseif isfield(file,'skin')
        scalp=file.skin;
    else
        error('BAD SURFACE SELECTION');
    end;
    clear file
end


%% Surface DownSampling
if SurfaceDwnSmpl
    np=10000;
    oldScalp=scalp;
    clear scalp;
    %Surface Triangles Reduction
    [scalp.tri scalp.vert]=reducepatch(oldScalp.tri,oldScalp.vert,np);
    clear np
end;


%% Ch Positions processing
%importing in workspace electrode positions
[orgPOS]=LoadElPosFile(posFullFileName);
%validating electrode position
POS=ValidatePos(orgPOS);
clear orgPOS
%valid channels initialization
vc=1:size(POS.elCoords,1);
%valid channels selection
str=POS.elLbls;
s = [1:length(str)];
v = 1;
% [s,v]=listdlg('PromptString','Select valid channels:','SelectionMode','multiple','InitialValue',vc,'ListString',str);
clear str
if ~v
    return;
end;
clear v
%valid channels updating
vc=s;
clear s
%valid data selection
data=DataVect(vc,:);
clear DataVect
%updating POS with selected channels
if ~isempty(vc) && isnumeric(vc)
    POS.selChans=vc;
    POS.elCoords=POS.elCoords(vc,:);
    POS.elLbls=POS.elLbls(vc);
end;
clear vc
%setting referencies
scalp.ref=POS.refCoords;
scalp.refLbls=POS.refLbls;
%Ch Pos Projection
oldPOS=POS;
clear POS
projPOS=ResampMesh(scalp,oldPOS,CENTER);


%% Surface Vertices Processing
mapping=VerticesProcessing(projPOS,scalp);


%% Interpolation Matrix Computation
%viene replicata la posizione del centro per un numero di volte
%pari alle posizioni elettrodiche
posCenters=repmat(CENTER,[size(projPOS.elCoords,1) 1]);
%viene replicata la posizione del centro per un numero di volte
%pari ai vertici validi delle triangolazioni
scalpCenters=repmat(CENTER,[length(mapping.validVert) 1]);
%calcolo matrice di mappaggio: prende in ingresso le posizioni
%degli elettrodi rispetto al nuovo punto di osservazione, le
%posizioni dei vertici validi rispetto al nuovo punto di osservazione,
%lambda e modality
mapping.mapMat=sphInterpMat(projPOS.elCoords-posCenters,scalp.vert(mapping.validVert,:)-scalpCenters,lambda,mode);
clear posCenters scalpCenters lambda mode


%% Graphical part
%setting cbar limits {'auto' 'symauto' 'zeroauto' '+zeroauto' '-zeroauto'}
mapping.cLimMode ='symauto';
%drawing data on scalp
mapping.hndls=DrawScalp(scalp,mapping,data);
if CBAR
    colorbar
end;
% colormap(cmap('iris'))


%% add montage to surface
if ADDmnt
    hold on;
    %electrodes like colored circles
    elColor='k';
    addmontage(projPOS,'color',elColor);
    hold off;
end;


%% Texture mapping
if TEXTUREmapping
    % load earth X map% Load image data, X, and colormap, map
    load('C:\Programmi\MATLAB\R2006b\work\Matlab Struct\HEADtest.mat', 'head');
    imfilename = 'C:\Programmi\MATLAB\R2006b\work\Matlab Struct\image035.gif';
    [X,map] = imread(imfilename);
    % sphere; h = findobj('Type','surface');
    rotmat = rotmatrix([1 0 0], pi/12) * rotmatrix([0 0 1], -pi/2);
    [sph.x sph.y sph.z] = sphere(50);
    siz=size(sph.x);
    sph.vert = [sph.x(:) sph.y(:) sph.z(:)] * rotmat';
    sphead = resampmesh(head, sph, [0 0 0], true);
    sphead.x = reshape(sphead.vert(:, 1), siz);
    sphead.y = reshape(sphead.vert(:, 2), siz);
    sphead.z = reshape(sphead.vert(:, 3), siz);
    h = surface(sphead.x, sphead.y, sphead.z);
    % hemisphere = [ones(257,125),...
    %               X,...
    %               ones(257,125)];
    % set(h,'CData',flipud(hemisphere),'FaceColor','texturemap')
    set(h,'CData',flipud(X),'FaceColor','texturemap', 'Edgecolor', 'none')
    colormap(map)
    axis equal
    view([90 0])
    set(gca,'CameraViewAngleMode','manual')
    view([65 30])
end;






















%% OPTIONAL
% % % % % %trasformazione delle coordinate cartesiane dei vertici dello scalpo in
% % % % % %coordinate polari
% % % % % if ~isfield(head, 'uvr')
% % % % %     head.uvr=xyz2uvr(head.vert);
% % % % % end% if
% % % % % %trasformazione delle coordinate cartesiane delle posizioni degli elettrodi
% % % % % %in coordinate polari
% % % % % if ~isfield (POS, 'uvr')
% % % % %     POS.uvr=xyz2uvr(POS.elCoords);
% % % % % end% if
% % % % % %viene creata una triangolazione a partire dai dati angolari delle
% % % % % %coordinate polari (forse si ipotizza che i punti siano su una sfera,
% % % % % %abbiano cioè lo stesso raggio)
% % % % % if ~isfield(POS, 'tri')
% % % % %     POS.tri=delaunay(POS.uvr(:, 1), POS.uvr(:, 2));
% % % % % end% if
% % % % % %usando tsearch(x,y,TRI,xi,yi) a partire da una triangolazione TRI
% % % % % %effettuata con delaunay da x ed y, si ottengono gli indici di TRI che
% % % % % %corrispondono ai punti specificati da xi ed yi (il ragionamento è ancora
% % % % % %di tipo sferico, senza considerare il raggio delle coordinate)
% % % % % k = tsearch(POS.uvr(:, 1), POS.uvr(:, 2), POS.tri, ...
% % % % %     head.uvr(mapping.validVert, 1), head.uvr(mapping.validVert, 2));
% % % % % %poichè tsearch restituisce NaN per tutti i punti al di fuori del guscio
% % % % % %convesso, impongo per tutti i punti che non hanno NaN il peso pari ad 1
% % % % % mapping.vertWeight(~isnan(k))=1;



























% % % % % %disegna lo scalpo
% % % % % axHndl=axes;
% % % % % drawscalpR(BioSurface,axHndl);
% % % % % 
% % % % % %aggiunge montaggio sullo scalpo
% % % % % montColor='w'; %gli elettrodi appaiono come puntini neri
% % % % % hndls=addmontage(projPOS,axHndl,'color',montColor);
% % % % % 
% % % % % lambda=0;
% % % % % mode='sphinterp';
% % % % % mapping=mapinit(projPOS, BioSurface, mode, lambda);
% % % % % mapping.hndls=hndls;
% % % % % 
% % % % % mapping.cLimMode='symauto';
% % % % % mapping.colorMap='hsv'; %Per avere le mappe a colori
% % % % % %mapping.colorMap='-iris'; %Per avere le mappe a colori (nero max positivo e bianco max negativo)
% % % % % %mapping.colorMap='-hsv'; %Per avere le mappe a colori
% % % % % %mapping.colorMap='-gray'; %Per avere le mappe in scala di grigi
% % % % % %mapping.colorMap='soft';
% % % % % %mapping.colorMap='reds';
% % % % % %mapping.colorMap='temperature';
% % % % % 
% % % % % [dataVal mapping]=updateScalp(data,mapping);
% % % % % 
% % % % % 
% % % % % 
% % % % % 
% % % % % 
% % % % % 
% % % % % 
% % % % % 
% % % % % 
% % % % % 
% % % % % 
% % % % % 
% % % % % 
% % % % % 
% % % % % 
% % % % % 
% % % % % 
% % % % % 
% % % % % 
% % % % % 
% % % % % 
% % % % % UD=get(axHndl,'UserData');
% % % % % UD.mapping=mapping;
% % % % % UD.data=data;
% % % % % set(axHndl, 'UserData', UD);
% % % % % 
% % % % % 
% % % % % 
% % % % % 
% % % % % 
% % % % % 
% % % % % 
% % % % % 
% % % % % 
% % % % % 
% % % % % 
% % % % % 
% % % % % 
% % % % % 
% % % % % 
% % % % % %%%%%%DA CONTROLLARE
% % % % % 
% % % % % 
% % % % % 
% % % % % 
% % % % % 
% % % % % figUD.elLbls=POS.elLbls;
% % % % % figUD.validChans=vc;
% % % % % set(gcf, 'color', 'w',...
% % % % %     'NumberTitle','off',...
% % % % %     'Name','Map',...
% % % % %     'Userdata',figUD); %con il fondo bianco
% % % % % 
% % % % % set(gca, 'visible', 'off', 'Tag', 'DM3');
% % % % % 
% % % % % %********disegna le etichette dei canali ****************
% % % % % th=tritext(1.05*projPOS.elCoords, projPOS.elLbls);
% % % % % set(th, 'color', 'k',...
% % % % %     'Visible','off', ...
% % % % %     'HorizontalAlignment', 'center', ...
% % % % %     'VerticalAlignment', 'middle');
% % % % % 
% % % % % menu1=uimenu(gcf,...
% % % % %     'Label','Function');
% % % % % menu2=uimenu(menu1,...
% % % % %     'Label','Electrodes',...
% % % % %     'Callback','shoelect');
% % % % % menu3=uimenu(menu1,...
% % % % %     'Label','ColorMap',...
% % % % %     'Callback','LimMap');
% % % % % menu4=uimenu(menu1,...
% % % % %     'Label','HalfHead',...
% % % % %     'Callback','LimMap1');
% % % % % menu5=uimenu(menu1,...
% % % % %     'Label','NewPot',...
% % % % %     'Callback','newpot');
% % % % % 
% % % % % view([-180 0]);
% % % % % 
% % % % % %ISTRUZIONI ACCESSORIE
% % % % % fh=gcf;
% % % % % ah=gca;
% % % % % UD=get(ah, 'UserData');
% % % % % UD.hndls.th=th;%hndl delle etichette dei canali;
% % % % % UD.hndls.menu=menu2;
% % % % % % % % delete(UD.hndls.rPHndls);
% % % % % % % % delete(UD.hndls.rLabHndls);
% % % % % % % % delete(UD.hndls.montRefPHndl);
% % % % % % % % delete(UD.hndls.montLabHndls);
% % % % % % % % delete(UD.hndls.montRefLabHndls);
% % % % % set(UD.hndls.montPHndl, 'markerSize', 3);
% % % % % set(ah,'Userdata',UD)
% % % % % axis tight
% % % % % drawnow