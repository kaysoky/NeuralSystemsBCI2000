%%%%%  applies common average reference to the channels
%%%%%  Dean Krusienski   1/2005              
%%%%%  Wadsworth Center/NYSDOH     
% (C) 2000-2008, BCI2000 Project
% http://www.bci2000.org

function CARSignal=CARfilter(signal)

fprintf(1, 'Applying CAR Filter...\n');

numch=size(signal,2);
CA=mean(signal,2);
CARSignal=signal-repmat(CA,1,numch);



% nch=size(signal,2);
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% exclude primary channel
% for kk=1:nch
%     CA=(nch/(nch-1))*(mean(signal,2)-(1/nch)*signal(:,kk));
%     CARSignal(:,kk)=signal(:,kk)-CA;    
% end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% include all channels
% CARSignal=zeros(size(signal,1),nch);
% CA=mean(signal,2);
% for kk=1:nch    
%     CARSignal(:,kk)=signal(:,kk)-CA;    
% end
