function hndls=addmontage(POS,varargin)

%disable ref view
hideREF=true;
%electrode labels translation factor
LBLFACTOR=1.1;
%electrode markers translation factor
MKRFACTOR=1.0;

argin=varargin;

%default settings
montColor='k';
refColor='g';

%getting elColor & elMarker
while ~isempty(argin)
    switch argin{1}
        case 'color'
            montColor=argin{2};
            argin([1 2])=[];
    end
end

%electrode positions & labels
elPOS=POS.elCoords;
elLabels=POS.elLbls;
%channels selection for visualization
s = [1:length(elLabels)];
v = 1;
% [s,v]=listdlg('PromptString','Select valid channels:','SelectionMode','multiple','ListString',elLabels);
clear str
if ~v
    return;
end;
clear v
%plot electrodes markers & labels
elHndl=plot3(MKRFACTOR*elPOS(s,1),MKRFACTOR*elPOS(s,2),MKRFACTOR*elPOS(s,3),'o');
hold on
elLabHndls=text(LBLFACTOR*elPOS(s,1), LBLFACTOR*elPOS(s,2), LBLFACTOR*elPOS(s,3), elLabels(s,1));
%setting markers % labels properties
set(elHndl,'MarkerEdgeColor',montColor,'MarkerSize',7);
set(elLabHndls,'HorizontalAlignment','center','VerticalAlignment','middle','FontName', 'Verdana')
%getting handles
hndls.elHndl=elHndl;
hndls.elLabHndls=elLabHndls;

if ~hideREF
    %ref points positions & labels
    refPOS=POS.refCoords;
    refLabels=POS.refLbls;
    %plot ref points markers & labels
    refHndl=plot3(refPOS(:,1),refPOS(:,2),refPOS(:,3),'o');
    refLabHndls=text(refPOS(:,1),refPOS(:,2),refPOS(:,3),refLabels);
    %setting markers % labels properties
    set(refHndl,'MarkerEdgeColor','k','MarkerFaceColor',refColor,'MarkerSize',10);
    set(refLabHndls,'HorizontalAlignment','center','VerticalAlignment','middle','FontName', 'Verdana')
    %getting handles
    hndls.refHndl=refHndl;
    hndls.refLabHndls=refLabHndls;
end;