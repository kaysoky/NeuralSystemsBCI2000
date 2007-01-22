% MU_SCRIPT example script for BCI2000 mu-rhythm analysis
% Usage: 
%        run "mario -gui" to setup the environment, and then execute this script
% Purpose:
%        Perform a typical analysis on data acquired with RJB or D2Box
%        applications. Process includes:
%        -  load an example dat file containing data acquired by BCI2000
%           during a D2box task;
%        -  select conditions (Targets) and time intervals assigned to either 
%           condition; 
%        -  extract spectral features; 
%        -  run a r-square analysis on this dataset.

% Authors: A. Emanuele Fiorilla; Febo Cincotti.

%  Copyright (C) 2007 Febo Cincotti, Fondazione Santa Lucia, Rome, Italy
%                     The BCI2000 Project, http://www.bci2000.org

%% Initialize
clear; clc

% This is the main variable of the analysis tool. It will contain all data
% and information necessary for completing the analysis and visualizing its
% results.
BCI2k = struct( ...
   'General',        'empty', ...
   'Files',          'empty', ...
   'Protocol',       'empty', ...
   'Conditioning',   'empty', ...
   'FeatExtraction', 'empty', ...
   'Statistics',     'empty' ...
   )

% Determine install directory
MarioFullFileName = which ('mario'); 
MarioPath = fileparts(MarioFullFileName);
DataPath = fullfile(MarioPath, 'TestData');


%% Select data files
% Two datasets are distributed with this package. The first is collected
% with scalp electrodes, the second is acquired subdurally. You can put
% here your own files. See "load_bcimnt.m" for help on how to create the
% corresponding montage file (*.mmf).

% ----------------EEG DATA------------------------
Dat_Path=fullfile(DataPath, 'EEG_Mu');%,...
Dat_Files={'CIFES008R04.dat'  'CIFES008R05.dat'};
Montage_File = fullfile(DataPath, 'EEG_Mu', 'CIFES008R04.mmf');
% -------------------------------------------------------------------------

% ----------------ECoG DATA-----------------------
% Montage_File=[];
% Dat_Files={'C:\Documents and Settings\pcabi\Desktop\DATA\ECoG\ECoG_Data\ESS001R07.dat',...
%     'C:\Documents and Settings\pcabi\Desktop\DATA\ECoG\ECoG_Data\ESS001R14.dat',...
%     'C:\Documents and Settings\pcabi\Desktop\DATA\ECoG\ECoG_Data\ESS001R21.dat'};
% -------------------------------------------------------------------------

[BCI2k.Files, BCI2k.General] = bci_loadfiles(Dat_Files, Montage_File, Dat_Path);
% clear Dat_Files;
% clear Mnt_File;
disp(BCI2k)

%% Define analysis protocol
% We prepare here the ground for two alternative types of analyses -
% standard and custom. In the first case, the protocol definition is
% encoded into a function that is distributed in this package. In the
% second case, the user can define some expression, which will be evaluated
% to determine how to split trials, which part of them to extract, how to
% label the data, etc.  See "bci_protocol.m" for further help


AnalysisName      = 'MuRhythm';% e.g. 'MuRhythm' or 'MyMu_Custom'
Targets           = [1 2];
IsStandardAnalysis  = isempty(strfind(lower(AnalysisName), '_custom'));

if IsStandardAnalysis
   % default analysis is implemented by a specific function
   BCI2k.Protocol = bci_protocol(BCI2k.Files.States, AnalysisName, Targets);
else
   % custom analysis by definition of custom virtual states
   switch lower(AnalysisName)
      case 'mymu_custom'
         StatesComm.ValidSamples = 'Feedback == true';
         StatesComm.Regressor    = 'TargetCode';
         StatesComm.Break        = '0';
         StatesComm.StimulusCode = 'TargetCode';
         StatesComm.TrialStart   = 'edge(~TargetCode)';
%       case 'p300_custom'
%          StatesComm = struct( ...
%             'ValidSamples', 'true', ...
%             'Regressor', 'StimulusCode', ...
%             'Break', 'false', ...
%             'Stimulus', 'Flashing', ...
%             'TrialStart', 'edge(PhaseInSequence == 3)' ...
%             );
      otherwise
         marioerror('unknownprotocol', 'Unknown protocol definition: %s', AnalysisName);
   end% switch
   % pass the custom definition as argument to bci_protocol
   BCI2k.Protocol = bci_protocol(BCI2k.Files.States, 'CustomAnalysis',StatesComm);
end% if custom protocol
disp(BCI2k)

%% Signal pre-processing (spatial and frequency filters)
BCI2k.Conditioning = bci_conditioning('CarFilt',BCI2k.Files.Data,BCI2k.Files.Geom);
disp(BCI2k)


%% Feature Extraction: autoregressive spectral estimation
SamplingFrequency = str2double(BCI2k.Files.Parameters.SamplingRate.Value{1});
FExParams =  struct( ...
   'EpochOverlap', 0.33, ...
   'EpochLength', 1, ...
   'Hp', 0, ...
   'Lp', 60, ...
   'Step', 0.2, ...
   'BW', 2, ...
   'Trend', 1, ...
   'Order', 16, ...
   'FSamp',  SamplingFrequency ...
   );
BCI2k.FeatExtraction = bci_featextractor('iterativemem','muepocher', ...
   FExParams, BCI2k.Conditioning.FilteredSignal, BCI2k.Protocol.States);
% clear FExParams;
disp(BCI2k)

%% Statistical comparison of EEG: Target 1 vs. Target 2
BCI2k.Statistics = bci_statanalysis('RSquare',BCI2k.FeatExtraction);
disp(BCI2k)


%% Visualize results of analysis
% BCI2k.Visualizer.FeatVisualizer.Colormap = 
% BCI2k.Visualizer.FeatVisualizer.Shading = 
% BCI2k.Visualizer.FeatVisualizer.ColorScaling.Min = 
% BCI2k.Visualizer.FeatVisualizer.ColorScaling.Max = 

BCI_FeatVisualizer(BCI2k,'TopographViewer','ComboViewer','SpectraViewer');

% BCI2k.Visualizer = flat_BCI_FeatVisualizer(BCI2k,'TopographViewer','ComboViewer','SpectraViewer');

