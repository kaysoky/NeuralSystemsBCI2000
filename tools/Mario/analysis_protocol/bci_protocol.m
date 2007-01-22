function [Protocol] = bci_protocol(States, Analysis, varargin)
% BCI_PROTOCOL define how to segment the dataset for the analysis
%
% TODO: help, licence
%
% Author: A. Emanuele Fiorilla

%  Copyright (C) 2007 Febo Cincotti, Fondazione Santa Lucia, Rome, Italy
%                     The BCI2000 Project, http://www.bci2000.org

FunctionString = sprintf('bci_%s', lower(Analysis));
if ~exist(FunctionString, 'file')% looks for an m-file with the same name of the analysis
    marioerror('nonexistentfunction', 'Protocol Analysis not found %s', FunctionString);
end
AnalysisFunc = str2func(FunctionString);
Protocol.States = AnalysisFunc(States, varargin{:});
Protocol.Type = Analysis
try
   Protocol.Definition = varargin{1}; % TODO: enhance readability
catch
   Protocol.Definition = 'Standard';
end% if