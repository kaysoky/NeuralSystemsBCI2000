function FilteringData = bci_carspilt(Data,Geom)
% BCI_CARSPFILT

ValidChansNum=sum(Geom.MMF.ValidChannels);
ChansNum=length(Geom.MMF.ValidChannels);

FiltMask= ~eye(ValidChansNum);

% TODO: restrict output channes to the valid channels list
FiltMatrix=ones(size(FiltMask));
FiltMatrix(FiltMask)=-1./(ValidChansNum);
FiltMatrix(~FiltMask)=1-1./(ValidChansNum);

SpatFiltMatrix=zeros(ChansNum,ValidChansNum);
SpatFiltMatrix(find(Geom.MMF.ValidChannels),:)=FiltMatrix;

FilteringData.SpatFiltMatrix=SpatFiltMatrix;
FilteringData.Geom = Geom;
% TODO: copy only information of mmf file relative to valid channels