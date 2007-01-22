function FeatExtraction = bci_featextractor(FEMethod, EpochMethod, FExParams, Data, States)
% BCI_FEATEXTRACTION feature extraction main module
% TODO: help, licence

% Authors: A. Emanuele Fiorilla; Febo Cincotti.

%  Copyright (C) 2007 Febo Cincotti, Fondazione Santa Lucia, Rome, Italy
%                     The BCI2000 Project, http://www.bci2000.org

%% Epocher setup
FunctionString = sprintf('bci_%s', lower(EpochMethod));
if ~exist(FunctionString, 'file')% looks for an m-file with the same name of the Epocher
   marioerror('nonexistentfunction', 'Epocher not found %s', FunctionString);
end% if
EpocherFunc = str2func(FunctionString);
EpochOverlap = FExParams.EpochOverlap;
if isfield(FExParams,'EpochLength')
    SampEpochLength = FExParams.EpochLength*FExParams.FSamp;
elseif isfield(FExParams,'SampEpochLength')
    SampEpochLength = FExParams.SampEpochLength;
end

%% Feature Extractor setup
FunctionString = sprintf('bci_%s', lower(FEMethod));
if ~exist(FunctionString, 'file')% looks for an m-file with the same name of the FE
   marioerror('nonexistentfunction', 'Feature Extractor not found %s', FunctionString);
end% if
FeatExtrFunc = str2func(FunctionString);
FeatExtraction.Params = FExParams;


%% Application to data

% Identify each chunk of data (epochs) on which the FE will iterate
EpochIndex = EpocherFunc(States, SampEpochLength, EpochOverlap);

% Collapse states into a single value per epoch
FeatExtraction.States = bci_statedecim(FEMethod,States,EpochIndex);

% Application of FE to data
FeatExtraction.Data = FeatExtrFunc(Data,FExParams,EpochIndex);
