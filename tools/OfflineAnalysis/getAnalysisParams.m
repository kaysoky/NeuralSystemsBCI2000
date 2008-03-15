%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% $Id: getAnalysisParams.m 2007-111-26 12:31:37EST fialkoff $ 
%% 
%% File: getAnalysisParams.m 
%% 
%% Author: Joshua Fialkoff <fialkj@rpi.edu>, Gerwin Schalk <schalk@wadsworth.org>
%%
%% Description: This function returns an empty analysis params structure.
%%
%% (C) 2000-2008, BCI2000 Project
%% http:%%www.bci2000.org 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function analysisParams = getAnalysisParams(domain, acqMode, dataFiles)
% getAnalysisParams     returns an empty structure for use with
% runBasicAnalysis.  The dataFile field is set equal to the value of the
% dataFile input argument

error(nargchk(0,3,nargin));


analysisParams = struct();
switch nargin
case 3
  if iscell(dataFiles)
    analysisParams.dataFiles = dataFiles;
  else
    analysisParams.dataFiles = {dataFiles};
  end
case 2
  analysisParams.dataFiles = {};
case 1
  analysisParams.dataFiles = {};
  acqMode = '';
case 0
  analysisParams.dataFiles = {};
  acqMode = '';
  domain = '';
end

analysisParams.targetConditions = {};
analysisParams.conditionLabels = {};
analysisParams.trialChangeCondition = '';
analysisParams.spatialFilt = '';
analysisParams.montageFile = '';
analysisParams.domain = domain;
analysisParams.acqType = acqMode;
analysisParams.channels = [];
analysisParams.topoParams = [];
  
