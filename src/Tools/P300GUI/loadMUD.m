function [MUD]=loadMUD(dir1,MUDfile)
    
% [MUDfile dir1]=uigetfile('*.mud','yo')

% MUD.trainfile=dlmread([dir1 MUDfile],' ',[1 0 1 0])
FID = fopen([dir1 MUDfile]);
sdat=fscanf(FID,'%s');
ind=findstr('Size',sdat);
ind2=findstr('softwarech',sdat);

if ~isempty(ind2)
    vect=dlmread([dir1 MUDfile],' ',[4 0 4 5]);
else
    vect=dlmread([dir1 MUDfile],' ',[4 0 4 4]);
end

MUD.MUD=dlmread([dir1 MUDfile],' ',16,0);
numchannels=vect(5);
MUD.channels=dlmread([dir1 MUDfile],' ',[13 0 13 numchannels-1]);
if ~isempty(ind)
    wln=dlmread([dir1 MUDfile],' ',[10 0 10 0]);
    MUD.windowlen=[0 wln];
else
    MUD.windowlen=dlmread([dir1 MUDfile],' ',[10 0 10 1]);
end
MUD.MA=vect(3);
MUD.DF=vect(4);
MUD.SF=vect(1);
MUD.RS=vect(2);
MUD.trainfile=MUDfile;
MUD.smprate=dlmread([dir1 MUDfile],' ',[7 0 7 0]);

if ~isempty(ind2)    
    MUD.softwarech=vect(6);
else
    MUD.softwarech=[];
end






