function Vinfo = VerticesProcessing (POS,surface)


%% costants definition
%factors for importance treshold computation:
%['fully important' 'unimportant']
THRESHFACTOR=[0.2 1.5];


%% importance treshold computation
%posizioni elettrodiche
pos=POS.elCoords;
%numero di elettrodi
nEl=size(pos,1);
%per ogni elettrodo viene calcolata la distanza (al quadrato) dall'elettrodo
%più vicino
repNEl=ones([nEl-1 1]);
clear minSqDst;
for elA=1:nEl
    elAs=elA(repNEl);
    elBs=(1:nEl)';
    elBs(elA)=[];
    minSqDst(elA,1)=min(sum((pos(elAs,:)-pos(elBs,:)).^2,2));
end
clear elA
clear repNEl elAs elBs
%la distanza maggiore tra quelle tra elettrodi vicini appena calcolate è
%utilizzata per definire il limite superiore per la piena rilevanza (0.2 d)
%ed il limite inferiore per l'irrilevanza (1.5 d)
threshDst=THRESHFACTOR*max(minSqDst);
clear minSqDst

%% weight computation for surface vertices
%numero di vertici
nVert=size(surface.vert,1);
Vinfo.nVert=nVert;
repNV=ones([nVert 1]);
clear sqDistMat;
%per ogni elettrodo viene calcolata la distanza (al quadrato) da tutti i
%vertici della struttura: i dati relativi a ciascun elettrodo occupano una
%intera colonna della matrice sqDistMat
for el=1:nEl
    els=el(repNV);
    elPoss=pos(els,:);
    sqDistMat(:,el)=sum((surface.vert-elPoss).^2,2);
end
clear el
clear els elPoss repNV pos
%per ogni vertice considero solo la distanza dall'elettrodo più vicino: il
%vettore colonna d2, quindi, contiene le distanze tra ciascun vertice e
%l'elettrodo ad esso più vicino.
d2=min(sqDistMat,[],2);
clear sqDistMat
%vengono settati come validi solo quei vertici caratterizzati dalla
%distanza dall'elettrodo più vicino inferiore al valore di soglia di
%irrelevanza
Vinfo.validVert=find(d2<threshDst(2));
%calcolo dei pesi: la distanza caratteristica di ciascun vertice viene
%clippata tra i due valori di soglia, le viene sottratto il valore minimo
%di soglia e viene divisa per la differenza tra le due soglie.
%il complemento ad uno del numero risultante è il peso da assegnare al
%vertice
Vinfo.vertWeight=1-(clip(d2(Vinfo.validVert),threshDst(1),threshDst(2))-threshDst(1))/diff(threshDst);
clear d2