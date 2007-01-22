function SignedRSquare = bci_rsquare(Data, Regressor)
% BCI_RSQUARE 
%
% TODO: help, licence, simplify call sequence

% Author: A. Emanuele Fiorilla.

%  Copyright (C) 2007 Febo Cincotti, Fondazione Santa Lucia, Rome, Italy
%                     The BCI2000 Project, http://www.bci2000.org

PackedFeatures=reshape(Data,[size(Data,1)*size(Data,2) size(Data,3)]);

[ressq, sgnR2]= calc_rsqu_f(PackedFeatures,single(Regressor)');
SigRsq=ressq.*sgnR2;

SignedRSquare = reshape(SigRsq,[size(Data,1) size(Data,2)]);