%%%%%  function to plot the target and standard responses
%%%%%  Dean Krusienski   1/2005
%%%%%  Wadsworth Center/NYSDOH
% (C) 2000-2007, BCI2000 Project
% http://www.bci2000.org

function P300chtime(AllResponses,Type,windowlen,SamplingRate,tfile,SF)

totch=size(AllResponses,3);

AllResponses=single(AllResponses);

tpe=find(Type==1);
AvgTargets=mean(AllResponses(tpe,:,:));
tpe=find(Type==0);
AvgNTargets=mean(AllResponses(tpe,:,:));

clear AllResponses Type

AvgTargets=reshape(AvgTargets,windowlen,totch);
AvgNTargets=reshape(AvgNTargets,windowlen,totch);


warning off MATLAB:xlswrite:AddSheet

name=[tfile '_responses.xls'];

% xlswrite(name, {'Targets Cz'},tfile,'A1')
% xlswrite(name, {'Standards Cz'},tfile,'A2')
% xlswrite(name, {'Targets Pz'},tfile,'A3')
% xlswrite(name, {'Standards Pz'},tfile,'A4')
% 
% xlswrite(name, AvgTargets(:,11)',tfile,'B1')
% xlswrite(name, AvgNTargets(:,11)',tfile,'B2')
% xlswrite(name, AvgTargets(:,51)',tfile,'B3')
% xlswrite(name, AvgNTargets(:,51)',tfile,'B4')


xlswrite(name, AvgTargets',tfile,'B1')
xlswrite(name, AvgNTargets',tfile,'B68')
