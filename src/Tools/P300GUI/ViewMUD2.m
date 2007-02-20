% (C) 2000-2007, BCI2000 Project
% http://www.bci2000.org

rows=5;
columns=3;
[MUDfile,dir1]=uigetfile('*.mud','Select the mud file');

vect=dlmread([dir1 MUDfile],' ',[4 0 4 4]);
MUD.MUD=dlmread([dir1 MUDfile],' ',16,0);
numchannels=vect(5);
MUD.channels=dlmread([dir1 MUDfile],' ',[13 0 13 numchannels-1]);
MUD.windowlen=dlmread([dir1 MUDfile],' ',[10 0 10 0]);
MUD.MA=vect(3);
MUD.DF=vect(4);
MUD.SF=vect(1);
MUD.TF=vect(2);
MUD.trainfile=MUDfile;
MUD.smprate=dlmread([dir1 MUDfile],' ',[7 0 7 0]);


MUD.MUD=MUD.MUD(1:MUD.MA:size(MUD.MUD,1),:);

figure
set(gcf,'Name',[MUDfile])
nch=64;
steps=12;
gg=1;
for kk=1:steps:MUD.windowlen
  wts=zeros(1,nch);
  val=zeros(1,length(MUD.channels));
    for tt=1:length(MUD.channels)
        ind=find(MUD.MUD(:,2)>=kk & MUD.MUD(:,2)<kk+steps-1 & MUD.MUD(:,1)==tt);
        if ~isempty(ind)
            val(tt)=mean(MUD.MUD(ind,3));
        end
    end  
    
    if sum(val)~=0
    wts(MUD.channels)=val;
    subplot(rows,columns,gg)
    topoplotEEG(wts)
    title([num2str((kk+steps-1)/MUD.smprate*1000) ' ms'])
    gg=gg+1;
    end
    
end
end



