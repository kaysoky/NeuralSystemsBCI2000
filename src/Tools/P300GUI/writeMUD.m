function writeMUD(filename,dir1,MUD)
% (C) 2000-2007, BCI2000 Project
% http://www.bci2000.org

prmname=filename;
filename=[filename '.mud'];
numch=length(MUD.channels);

dlmwrite([dir1 filename],'Training File:',' ')
dlmwrite([dir1 filename],MUD.trainfile,'delimiter',' ','-append')
dlmwrite([dir1 filename],' ','delimiter',' ','-append')

dlmwrite([dir1 filename],'SF/BPF/MA/DF/#ch/softwarech:','delimiter',' ','-append')
dlmwrite([dir1 filename],[MUD.SF MUD.RS MUD.MA MUD.DF numch MUD.softwarech],'delimiter',' ','-append')
dlmwrite([dir1 filename],' ','delimiter',' ','-append')

dlmwrite([dir1 filename],'Sampling Rate:','delimiter',' ','-append')
dlmwrite([dir1 filename],MUD.smprate,'delimiter',' ','-append')
dlmwrite([dir1 filename],' ','delimiter',' ','-append')

dlmwrite([dir1 filename],'Window Begin/End:','delimiter',' ','-append')
dlmwrite([dir1 filename],MUD.windowlen,'delimiter',' ','-append')
dlmwrite([dir1 filename],' ','delimiter',' ','-append')

dlmwrite([dir1 filename],'Channels:','delimiter',' ','-append')
dlmwrite([dir1 filename],MUD.channels,'delimiter',' ','-append')
dlmwrite([dir1 filename],' ','delimiter',' ','-append')

dlmwrite([dir1 filename],'MUD:','delimiter',' ','-append')
dlmwrite([dir1 filename],MUD.MUD,'delimiter',' ','-append')






