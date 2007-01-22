function FeatMatr = bci_iterativemem(Data,FExParams,EpochIndex)
% BCI_ITERATIVEMEM
%
% TODO: help, licence

% Author: A. Emanuele Fiorilla.

%  Copyright (C) 2007 Febo Cincotti, Fondazione Santa Lucia, Rome, Italy
%                     The BCI2000 Project, http://www.bci2000.org

FEParmsVect = [FExParams.Hp FExParams.Lp FExParams.Step FExParams.BW FExParams.Trend FExParams.Order FExParams.FSamp];
for EpochNum = 1:size(EpochIndex,2)
    [Features, freq_bins]=mem(double(Data(EpochIndex(1,EpochNum):EpochIndex(2,EpochNum),:)),FEParmsVect);
    FeatMatr(:,:,EpochNum)=Features;
end