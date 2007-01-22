function Statistics = bci_statanalysis(StatMethod, FeatExtraction)
% BCI_STATANALYSIS 
%
% TODO: help, licence, simplify call sequence

% Author: A. Emanuele Fiorilla.

%  Copyright (C) 2007 Febo Cincotti, Fondazione Santa Lucia, Rome, Italy
%                     The BCI2000 Project, http://www.bci2000.org

%% Initialization
Data = FeatExtraction.Data;
Regressor = FeatExtraction.States.Regressor;

FunctionString = sprintf('bci_%s', lower(StatMethod));
if ~exist(FunctionString, 'file')% looks for an m-file with the same name of the stat
   marioerror('nonexistentfunction', 'Statistic not found %s', FunctionString);
end% if
StatFunc = str2func(FunctionString);
Statistics.Params.Analysis = StatMethod;

%% compute statistics
Statistics.Data = StatFunc(Data, Regressor);
