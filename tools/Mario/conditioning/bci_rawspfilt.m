function FilteringData = bci_rawspfilt(Data,Geom)
% BCI_RAWSPFILT

ValidChansNum=sum(Geom.MMF.ValidChannels);
ChansNum=length(Geom.MMF.ValidChannels);

FiltMatrix=ones(size(FiltMask));

% TODO: restrict output channes to the valid channels list
SpatFiltMatrix=zeros(ChansNum,ValidChansNum);
SpatFiltMatrix(find(Geom.MMF.ValidChannels),:)=FiltMatrix;

FilteringData.SpatFiltMatrix=SpatFiltMatrix;
FilteringData.Geom = Geom;
% TODO: copy only information of mmf file relative to valid channels