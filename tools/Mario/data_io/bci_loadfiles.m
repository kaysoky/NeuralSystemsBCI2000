function [Files, General] = bci_loadfiles(Dat_Files,Mnt_File, Dat_Path)
% BCI_LOADFILES
% TODO: help, copyright, licence

% Author: A. Emanuele Fiorilla

if nargin<2 || isempty(Dat_Path)
   Dat_Path = pwd;
end% if

   
%% Data files loading...
if ~isempty(Dat_Files)
    signal=[];
    states=[];
    parameters=[];
    Dat_Files_bak = Dat_Files;% for debugging purposes...
    % TODO: finer congruence check on data (channels, states, etc).
    for file_num = 1:length(Dat_Files)
       % check if filename has complete path, or needs be prepended by
       % Dat_Path
       temppath = fileparts(Dat_Files{file_num});
       if isempty(temppath)
          Dat_Files{file_num} = fullfile(Dat_Path, Dat_Files{file_num});
       end% fi
       % load data
       [sig,stats,params ] = load_bcidat(Dat_Files{file_num}); %data subset (1 run)
        signal=cat(1,signal,sig);        %Signal accumulator

        %states and parameters set
        if (file_num == 1)
            states = stats;
            parameters = params;        %Parameters from the first dat
        else
            stat_names = fieldnames(stats);
            for stat_ind = 1: length(stat_names)
                states.(stat_names{stat_ind}) = cat(1,states.(stat_names{stat_ind}),stats.(stat_names{stat_ind}));
            end
        end
    end
end

%Montage files loading...
if ~isempty(Mnt_File)
    MMF = load_bcimnt(Mnt_File);
else
    MMF.ValidChannels = ones(1,size(signal,2));
end


%% Preconditioning...data selection
%--------------------------------
%--------------------------------


%Global variables set...
%Files DAT section
Files.FileNames.Dats = Dat_Files;
Files.FileNames.Montage = Mnt_File; % TODO: check that this is full path 
Files.Data = signal;
Files.States = states;
Files.Parameters = parameters;
%Files MMF section
Files.Geom.MMF = MMF;
%General Info Section
General.Files.Dat_Files=Dat_Files;
General.Files.Mnt_File=Mnt_File;










%----------------------------