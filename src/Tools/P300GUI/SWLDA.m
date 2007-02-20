function [MUD]=SWLDA(Responses,Type,MAfilter,DecFact,windowlen,channels,SF,RS,trainfile,smprate,penter,premove,maxiter,softwarech,method)
% (C) 2000-2007, BCI2000 Project
% http://www.bci2000.org

numchannels=size(Responses,3);
Type=double(Type);
fprintf(1, 'Averaging and Decimating...\n');
first=ceil(((MAfilter-1)/DecFact)+1);
xx=size(Responses,1);
sizex=(length(1:DecFact:[windowlen(2)-windowlen(1)])-first+1)*numchannels;
tresponse=zeros(xx,sizex);
for hh=1:xx
    dresponse=filter(ones(1,MAfilter)/MAfilter,1,Responses(hh,:,:));
%     dresponse=filter(window(@hanning,MAfilter),1,Responses(hh,:,:));
    dresponse=dresponse(1,1:DecFact:[windowlen(2)-windowlen(1)],:);
    tresponse(hh,:)=reshape(dresponse(1,first:size(dresponse,2),:),1,sizex);
end

target=find(Type==1);
standard=find(Type==0);

% y=randsample(length(standard),length(target));

indtn=[target; standard];%(y)];
data=tresponse(indtn,:);
Label=2*(Type(indtn)-.5);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%% stepwise LDA
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
switch method
    case 1 %SWLDA
        fprintf(1, 'Stepwise Regression...\n');
        [B,SE,PVAL,in] = stepwisefit(data,Label,'maxiter',maxiter,'display','off','penter',penter,'premove',premove);% 'penter',.1,'premove',.15
    case 2 % Least Squares
        fprintf(1, 'Least Squares Regression...\n');
        B=regress(Label,[data ones(1,size(data,1))']);
        B=B(1:length(B)-1);
        in=ones(1,length(B));
    case 3 % Logistic
        fprintf(1, 'Logistic Regression...\n');
        B=robustfit(data,Label,'logistic');
        B=B(2:length(B));
        in=ones(1,length(B));
end

index=find(in~=0);
Variables=B(index);
Variables=10*Variables/max(abs(Variables));
chin=reshape(in,length(in)/numchannels,numchannels);
[samp,ch]=find(chin==1);
chused=unique(ch);
newind=windowlen(1)+(samp+first-2)*DecFact;
for rr=1:length(chused);
    idx=find(ch==chused(rr));
    ch(idx)=rr*ones(1,length(idx));
end

MUD.MUD=[];
hh=1;
for gg=1:size(Variables,1)
    MUD.MUD(hh:hh+MAfilter-1,1)=ch(gg)*ones(1,MAfilter);
    MUD.MUD(hh:hh+MAfilter-1,2)=newind(gg)-MAfilter+1:newind(gg);
    MUD.MUD(hh:hh+MAfilter-1,3)=Variables(gg)*ones(1,MAfilter);
    hh=hh+MAfilter;
end

if isempty(MUD.MUD)==1
    fprintf(1,'*** Unable to generate usable weights, try different parameters or dataset ***\n');
end

MUD.channels=channels(chused);
MUD.windowlen=windowlen;
MUD.MA=MAfilter;
MUD.DF=DecFact;
MUD.SF=SF;
MUD.RS=RS;
MUD.trainfile=trainfile;
MUD.smprate=smprate;
MUD.softwarech=softwarech;
fprintf(1, '...Done\n\n');
