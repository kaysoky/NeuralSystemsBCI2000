function EEG = BCI2000import

addpath(genpath('..\..\functions'));
EEG = [];
%% load the data
[fname, fpath] = uigetfile('*.dat','multiselect','on');
if fpath == 0
    return;
end

if ~iscell(fname)
    tmp = fname;
    clear fname;
    fname{1} = tmp; clear tmp;
end
for i=1:length(fname)
    fname{i} = [fpath,fname{i}];
end

files = struct('name',fname);
startPos = 0;
evCount = 1;
signal = [];
states = [];
[signal, states, parms] = load_bcidat(files.name);

stateNames = fieldnames(states);
commonStates = {'TargetCode','ResultCode','Dwelling','Feedback',...
    'IntertrialInterval'};
sel = [];
for i=1:length(stateNames)
    a = strFindCell(commonStates,stateNames{i},'exact');
    if a > 0
        sel = [sel, i];
    end
end
[sel, ok] = listdlg('liststring',stateNames,...
    'selectionmode','multiple','promptstring','Select BCI Events','initialvalue',sel);
if ok == 0
    data = [];
    return;
end
selectedEvents = stateNames(sel);
for k=1:length(stateNames)
    if ismember(stateNames{k}, selectedEvents)
        continue;
    end
    states = rmfield(states, stateNames{k});
end
states = compressData(states);
stateNames = fieldnames(states);

EEG = eeg_emptyset;
EEG.srate = str2num(parms.SamplingRate.Value{1});
EEG.nbchan = size(signal,2);
EEG.pnts = size(signal,1);
EEG.trials = 1;
EEG.data = signal';
EEG = eeg_checkset(EEG);

evCount = 1;
for i=1:length(stateNames)
    st = getfield(states,stateNames{i});
    for ev=1:length(st.latency)
        EEG.event(evCount).latency = st.latency(ev);
        EEG.event(evCount).position = st.value(ev);
        EEG.event(evCount).type = stateNames{i};
        evCount = evCount+1;
    end
end

EEG = eeg_checkset(EEG, 'eventconsistency');
EEG = pop_editeventvals(EEG, 'sort', {'latency',[0]});

%------------------------------
%-----------------------------------------------------------------------
function pos = strFindCell(str, toFind, option)

pos = 0;
if ~exist('option')
    option = 'all';
end

if ~iscellstr(str)
    error('Input to strFindCell must be a cell array of strings.');
    pos = 0;
    return;
end

switch option
    case 'all'
        count = 1;
        for i=1:length(str)
            if ~isempty(strfind(str{i}, toFind))
                pos(count) = i;
                count = count+1;
            end
        end

    case 'exact'
        for i=1:length(str)
            if (strcmp(str{i}, toFind))
                pos = i;
                return;
            end
        end
end

% -------------------------------------------------------------
function data = compressData(data, name)

compressStates = {'TargetCode','ResultCode','IntertrialInterval','Feedback','Dwelling'};

if isstruct(data)
    for i=1:length(compressStates)
        if isfield(data,compressStates{i})
            value = getfield(data,compressStates{i});
            if isstruct(value) continue; end
            data = setfield(data, compressStates{i}, compressData(value,compressStates{i}));
            clear value;
        end
    end
else
    if ~strFindCell(compressStates, name,'exact')
        return;
    end
    
    lat = find(diff(data) > 0) + 1;
    pos = data(lat);

    dur = [];
    for i=1:length(lat)
        b = find(data(lat(i):end) == 0);
        if ~isempty(b)
            dur(i) = b(1)-1;
        else
            dur(i) = length(data)-lat(i);
        end
    end
    clear data;
    data.latency = lat;
    data.value = pos;
    data.duration = dur(:);
end
