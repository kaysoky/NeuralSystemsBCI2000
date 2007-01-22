function [POS] = LoadElPosFile(inFileName)

%   imports in workspace electrode position recorded in a ".pos" or ".dig"
%   file

%   [POS] = LoadElPosFile ('fileName.pos')
%        POS.elCoords are (nElec x 3) arrays
%        POS.elLbls are (nElec x 3) cell arrays of strings
%        POS.refCoords are (nRef x 3) arrays
%        POS.refLbls are (nRef x 3) arrays of strings
%        POS.numElect

if nargin<1
    %input file selection
    [fName,path]=uigetfile({'*_ATT.dig';'*.pos'},'SELECT ELECTRODE POSITION FILE');
    if fName==0
        return
    end
    inFileName=fullfile(path,fName);
    clear path fName
end

%checking file ext
[auxP,auxN,EXT] = fileparts (inFileName);
clear auxP axN

%file opening
fid=fopen(inFileName);
if fid==-1
    return
end

switch EXT
    case '.dig'
        %loading electrodes coords & lbls
        [elLbls elCoords(:,1) elCoords(:,2) elCoords(:,3)]=textread(inFileName,'%s %f %f %f');
        numElectr=size(elLbls);
        %POS struct definition
        POS.elCoords=elCoords;
        POS.elLbls=elLbls;
        POS.numElectr=numElectr;
    case '.pos'
        %loading ref coords & lbls
        refLbls={};
        refCoords=[];
        headNum=fscanf(fid,'%f',1);
        while headNum==-1
            refLbls=[refLbls,{fscanf(fid,'%s',1)}];
            tmpCoord=fscanf(fid,'%f',3);
            if ~any(tmpCoord)
                tmpCoord=repmat(nan,size(tmpCoord));
            end
            refCoords=[refCoords;tmpCoord'];
            clear tmpCoord;
            headNum=fscanf(fid,'%f',1);
        end
        %loading electrodes coords & lbls
        numElectr=headNum;
        for curElec=1:numElectr
            headNum=fscanf(fid,'%f',1);
            elLbls{curElec}=fscanf(fid,'%s',1);
            tmpCoord=fscanf(fid,'%f',3);
            if ~any(tmpCoord)
                tmpCoord=repmat(nan,size(tmpCoord));
            end
            elCoords(curElec,:)=tmpCoord';
            clear tmpCoord;
        end
        clear curElec headNum
        %POS struct definition
        POS.elCoords=elCoords;
        POS.elLbls=elLbls;
        POS.refCoords=refCoords;
        POS.refLbls=refLbls;
        POS.numElectr=numElectr;
end;

%file closing
fclose(fid);
clear fid