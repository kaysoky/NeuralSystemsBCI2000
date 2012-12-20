function EEG = pop_loadBCI2000(fileName, events)
% pop_loadBCI2000() - loads BCI2000 .dat files into EEGLAB
%
% Usage:
%   >> EEG = pop_loadBCI2000();
%   >> EEG = pop_loadBCI2000(fileName);
%   >> EEG = pop_loadBCI2000(fileName, events);
%
% Inputs:
%   fileName - File name(s), either a string (possibly containing wildcards) or
%              a cell array of strings containing multiple file names
%   events   - List of events to import, can either be numeric (a vector) or a
%              cell array of strings containing the names of the events to
%              import (default: all events are imported)
%
% Outputs:
%   EEG      - EEGLAB data structure
%
% Examples:
%   Open GUI dialog:
%     >> EEG = pop_loadBCI2000();
%
%   Load all .dat files in the current folder with all events:
%     >> EEG = pop_loadBCI2000('*.dat');
%
%   Load all .dat files with event numbers 2 and 5:
%     >> EEG = pop_loadBCI2000('*.dat', [2, 5]);
%
%   Load all .dat files with events 'StimulusCode' and 'Feedback':
%     >> EEG = pop_loadBCI2000('*.dat', {'StimulusCode', 'Feedback'});
%
%   Load two specific files:
%     >> EEG = pop_loadBCI2000({'set001.dat', 'set002.dat'});

% Copyright by Clemens Brunner <clbrunner@ucsd.edu>
% Revision: 0.32
% Date: 12/20/2012
% Parts based on BCI2000import.m from the BCI2000 distribution (www.bci2000.org)

% Revision history:
%   0.32  Added compatibility for cell array of filenames
%         Removed loops and other simplifications.
%         Keeps any state change to non-zero value as an event.
%         -Chadwick Boulay boulay@bme.bio.keio.ac.jp        
%   0.31  Loads calibrated data
%   0.30: Create boundary events when loading multiple files
%   0.26: Import channel labels
%   0.25: Added EEG.urevent structure
%   0.20: Adapted to EEGLAB conventions
%   0.10: Initial version

% This program is free software; you can redistribute it and/or modify it under
% the terms of the GNU General Public License as published by the Free Software
% Foundation; either version 2 of the License, or (at your option) any later
% version.
%
% This program is distributed in the hope that it will be useful, but WITHOUT
% ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
% FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
%
% You should have received a copy of the GNU General Public License along with
% this program; if not, write to the Free Software Foundation, Inc., 59 Temple
% Place - Suite 330, Boston, MA  02111-1307, USA.

if nargin < 1  % No input arguments specified, show GUI
    [fileName, filePath] = uigetfile('*.dat', 'Choose BCI2000 file(s) -- pop_loadBCI2000', 'multiselect', 'on');
    
    if ~iscell(fileName)  % If only one file was selected
        files = struct('name', fullfile(filePath, fileName));
    else  % Multiple files were selected
        files = struct('name', fileName);
        for k = 1:length(files)
            files(k).name = fullfile(filePath, files(k).name);
        end
    end
    [ signal, states, parameters, ~, file_samples ] ...
        = load_bcidat( files.name, '-calibrated' );
    
    % Build a string consisting of all events, separated by a "|" (necessary for
    % inputgui() below)
    eventList = fieldnames(states);
    eventString = sprintf('%s|', eventList{:});
    eventString(end) = [];
        
    temp = inputgui('geometry', {1, 1}, 'geomvert', [1, 10], ...
        'uilist', {{'style', 'text', 'string', 'Select events'}, ...
        {'style', 'listbox', 'string', eventString, 'min', 0, 'max', 2}});
    
    if isempty(temp)  % Abort if user clicked "Cancel"
        error('Loading aborted.'); end
    
    events = eventList(temp{1});
    clear('temp');
    
else % fileName specified
    if iscell(fileName)  % Multiple file names in a cell array
        files = cell2struct(fileName, 'name', 1);
    else  % Just a string (possibly containing wildcards)
        files = dir(fileName);
        for k = 1:length(files)
            files(k).name = fullfile(fileparts(fileName), files(k).name);  % Add full path to each file name
        end
    end
    [ signal, states, parameters, ~, file_samples ] ...
        = load_bcidat(files.name, '-calibrated');
    
    if ~exist('events', 'var')  % If no events were specified, select all events contained in the file
        events = fieldnames(states); end
end

% Determine first sample index of individual files
if length(files)>1
    fileBorders = [1 1+cumsum(double(file_samples(1:end-1)))];
end

%Remove events (states) that were not selected.
stateNames = fieldnames(states);
[~, ia, ib] = setxor(events, stateNames);
if ~isempty(ia)
    warning('''%s'' is not an event in the signal. Therefore, this event is being ignored.\n',...
        events{ia});
end
states = rmfield(states, stateNames(ib));

states = compressData(states);
stateNames = fieldnames(states);

EEG = eeg_emptyset();
EEG.setname = 'Imported BCI2000 data set';
EEG.srate = parameters.SamplingRate.NumericValue;
EEG.nbchan = size(signal, 2);
EEG.pnts = size(signal, 1);
EEG.trials = 1;
EEG.data = signal';
EEG.chanlocs = struct('labels', parameters.ChannelNames.Value);% Assign channel labels
EEG = eeg_checkset(EEG);

for k = 1:length(stateNames)
    these_events = struct(...
        'latency', num2cell(states.(stateNames{k}).latency),...
        'position', num2cell(states.(stateNames{k}).value),...
        'duration', num2cell(states.(stateNames{k}).duration));
    [these_events.type] = deal(stateNames{k});
    EEG.event = [EEG.event these_events'];
end

if exist('fileBorders', 'var')  % If multiple files were loaded and concatenated, create boundary events at the start of new files
    EEG.event = eeg_insertbound(EEG.event, EEG.pnts * EEG.trials, fileBorders');  end

EEG = eeg_checkset(EEG, 'eventconsistency');
EEG = eeg_checkset(EEG, 'makeur'); %EEG.urevent
EEG = eeg_checkset(EEG); %EEG.urevent


function data = compressData(data)
% Compresses event data time series by noting only changes in event values.
%
% TODO: Detailed description

% List of known event types to compress
compressStates = {'TargetCode','ResultCode','IntertrialInterval','Feedback','Dwelling','StimulusCode'};

if isstruct(data)
    stateNames = fieldnames(data);
    for k = 1:length(stateNames)
        if ismember(stateNames{k}, compressStates)  % If the event is in the list of known events, compress it
            if ~isstruct(data.(stateNames{k}))
                data.(stateNames{k}) = compressData(data.(stateNames{k}));
            end
        else  % If the event is unknown, try to figure out if to compress it
            value = data.(stateNames{k});
            if isstruct(value)  % I don't know when this field can be a struct; obviously, it is ignored then
                continue; end
            % Specifies the percentage of number of different event values relative to the length of the EEG signals (e.g. 0.05 is 5%)
            relativeEventLength = 0.05;
            U = unique(value);
            
            % Compress only if there are at least 2 different event values AND
            % if the number of different event values does not exceed the above set threshold
            if length(U) > 1 && length(U) < length(value)*relativeEventLength
                data.(stateNames{k}) = compressData(value);
            else
                warning('''%s'' is not a valid event for EEGLAB. This event is being discarded.', stateNames{k});
                data = rmfield(data, stateNames{k});
            end
        end
    end

else
    
    data = double(data);
    ev_ch_bool = [false;diff(data) ~= 0]; %Any event change.
    ev_ch_ix = find(ev_ch_bool); %Sample indices for event changes.
    dur = [diff(ev_ch_ix);length(data)-ev_ch_ix(end)]; %Duration between event changes.
    pos = data(ev_ch_ix); %New value at event change.
    
    %Presumably we don't want to store events when the event changes to 0.
    keep_event = pos~=0;
    
    clear data;
    data.latency = ev_ch_ix(keep_event);
    data.value = pos(keep_event);
    data.duration = dur(keep_event);
end
