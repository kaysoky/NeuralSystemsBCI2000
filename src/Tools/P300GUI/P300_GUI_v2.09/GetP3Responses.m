function [Responses]=GetP3Responses(signal,trialnr,windowlen,StimulusCode,StimulusType,Flashing,channels,SF,rndsmp)

fprintf(1, 'Collecting Responses...\n');

if length(windowlen)==1
    windowlen(2)=windowlen; 
    windowlen(1)=0;     
end

numchannels=size(signal,2);
lenflash=length(Flashing);
ind=find(Flashing(1:lenflash-1)==0 & Flashing(2:lenflash)==1)+1;
xx=length(ind);

if rndsmp~=100
   numsmp=floor(xx*rndsmp/100); 
   vv=randsample(1:xx,numsmp);
   ind=ind(vv);
   xx=length(ind);
end

Responses.Responses=zeros(xx,windowlen(2)-windowlen(1),numchannels,'single');
for kk=1:xx
   Responses.Responses(kk,:,:)=signal(ind(kk)+windowlen(1)-1:ind(kk)+windowlen(2)-2,:);%-repmat(mean(signal(ind(kk)-1:ind(kk)+windowlen-2,:),1),windowlen,1);
end

Responses.Code=StimulusCode(ind);
Responses.Type=StimulusType(ind);
Responses.trial=trialnr(ind);
Responses.channels=channels;
Responses.SF=SF;

