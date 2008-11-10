function res = BCItimingAnalysis
fpath = uigetdir;
if fpath == 0 return; end

fList = [];
fList = getFileList(fpath, fList);
taskDefs = loadTaskFile;
if isempty(taskDefs) return; end
res = [];
for i=1:length(fList)
    fprintf('Analyzing %d/%d: %s\n',i, length(fList),fList{i});
    res{i} = doThreshAnalysis(fList{i}, taskDefs);
end

%-------------------------------------------
function res = doThreshAnalysis(fname, taskDefs)
res = [];
[signal,states,parms] = load_bcidat(fname);
ignoreDur = 5;

thisTask = [];
for i=1:length(taskDefs)
    statesFound = 1;
    parmsFound = 1;
    nameFound = strcmp(parms.SubjectName.Value{1},taskDefs{i}.name);
    stateNames = fields(states);
    if ~nameFound continue; end
    for s=1:length(taskDefs{i}.states)
        statesFound = ismember(taskDefs{i}.states, stateName) && statesFound;
    end
    if statesFound && parmsFound && nameFound
        thisTask = taskDefs{i};
        break;
    end
end
if isempty(thisTask) res.name = parms.SubjectName.Value{1}; return; end
%if thisTask.skip res.name = parms.SubjectName.Value{1};return; end
thresh = 5e4;
SBS = parms.SampleBlockSize.NumericValue;
ignoreDur = ignoreDur*SBS;
fs = parms.SamplingRate.NumericValue;
res.name = thisTask.name;
res.fs = fs;
res.SBS = SBS;
res.amp = [];
if thisTask.amp.flag
    a = find(diff(signal(:,thisTask.amp.ch+1)) >= std(diff(signal(:,thisTask.amp.ch+1))))+1;
    a(find(a <= ignoreDur)) = [];
    b = find(diff(a) < SBS/3)+1;
    a(b) = [];
    res.amp.vals = 1000*mod(a-1, SBS)/fs;
    res.amp.ts = a;
end

res.dAmp = [];
if thisTask.dAmp.flag
    a = find(signal(:,thisTask.dAmp.ch+1) == max(signal(:,thisTask.dAmp.ch+1)));
    a(find(a <= ignoreDur)) = [];
    b = find(diff(a) < SBS/3)+1;
    a(b) = [];
    res.dAmp.vals = 1000*mod(a-1, SBS)/fs;
    res.dAmp.ts = a;
end

res.vid = [];
if thisTask.vid.flag
    a = [];
    a = find(diff(signal(:,thisTask.vid.ch+1)) >= std(diff(signal(:,thisTask.vid.ch+1))))+1;
    a(find(a <= ignoreDur)) = [];
    b = find(diff(a) < SBS)+1;
    a(b) = [];
    state = double(getfield(states,thisTask.vid.state));
    st = [];
    for i=1:length(thisTask.vid.stateVal)
        st = [st;find(diff(state) == thisTask.vid.stateVal(i))+1];
    end
    st = sort(st);
    st(find(st <= ignoreDur)) = [];
    res.vid.vals = [];
    p = 1;
    for i=1:length(st)-1
        b = intersect(find(a >= st(i)), find(a < st(i+1)));
        if isempty(b) continue; end
        if length(b) > 1 b(2:end) = []; end
        res.vid.vals(p) = 1000*(a(b(1)) - st(i))/fs;
        res.vid.sigProc(p) = double(states.StimulusTime(st(i)) - states.SourceTime(st(i)));
        res.vid.jitter(p) = double(states.SourceTime(st(i)) - states.SourceTime(st(i)-SBS));
        res.vid.stim(p) = round(st(i)/SBS);
        p = p+1;
    end
    b = find(a >= st(end));
    if ~isempty(b) 
        res.vid.vals = [res.vid.vals, 1000*(a(b(1)) - st(end))/fs];
        res.vid.stim(end+1) = round(st(end)/SBS);
        res.vid.sigProc(end+1) = double(states.StimulusTime(st(end)) - states.SourceTime(st(end)));
        res.vid.jitter(end+1) = double(states.SourceTime(st(end)) - states.SourceTime(st(end)-SBS));
    end
    
%     
%     while ~isempty(a)
%         c = find(st >= a(1));
%         
%         if isempty(c) a = []; continue; end
%         res.vid.vals = [res.vid.vals, (st(c(1))-a(1))];
%         st(c(1)) = [];
%         a(1) = [];
%     end
end

res.aud = [];
if thisTask.aud.flag
    a = find(diff(signal(:,thisTask.aud.ch+1)) >= std(diff(signal(:,thisTask.aud.ch+1))))+1;
    a(find(a <= ignoreDur)) = [];
    b = find(diff(a) < SBS)+1;
    a(b) = [];
    state = double(getfield(states,thisTask.aud.state));
    st = [];
    for i=1:length(thisTask.aud.stateVal)
        st = [st;find(diff(state) == thisTask.aud.stateVal(i))+1];
    end
    st = sort(st);
    st(find(st <= ignoreDur)) = [];
    res.aud.vals = [];
    p = 1;
    for i=1:length(st)-1
        b = intersect(find(a >= st(i)), find(a < st(i+1)));
        if isempty(b) continue; end
        if length(b) > 1 b(2:end) = []; end
        res.aud.vals(p) = 1000*(a(b(1)) - st(i))/fs;
        res.aud.sigProc(p) = double(states.StimulusTime(st(i)) - states.SourceTime(st(i)));
        res.aud.jitter(p) = double(states.SourceTime(st(i)) - states.SourceTime(st(i)-SBS));
        res.aud.stim(p) = round(st(i)/SBS);
        p = p+1;
    end
    b = find(a >= st(end));
    if ~isempty(b) 
        res.aud.vals = [res.aud.vals, 1000*(a(b(1)) - st(end))/fs];
        res.aud.stim(end+1) = round(st(end)/SBS);
        res.aud.sigProc(end+1) = double(states.StimulusTime(st(end)) - states.SourceTime(st(end)));
        res.aud.jitter(end+1) = double(states.SourceTime(st(end)) - states.SourceTime(st(end)-SBS));
    end
    
%     
%     while ~isempty(a)
%         c = find(st >= a(1));
%         
%         if isempty(c) a = []; continue; end
%         res.aud.vals = [res.aud.vals, (st(c(1))-a(1))];
%         st(c(1)) = [];
%         a(1) = [];
%     end
end

res.sigProc = double(states.StimulusTime(1:SBS:end) - states.SourceTime(1:SBS:end));
a = find(res.sigProc < 0);
res.sigProc(a) = res.sigProc(a) + 2^15;

res.jitter = diff(double(states.SourceTime(1:SBS:end)));
a = find(res.jitter < 0);
res.jitter(a) = res.jitter(a)+2^16;
%res.jitter = res.jitter./(1000*SBS/fs);
%-------------------------------------------
function fileList = getFileList(fpath, fileList)
dlist = dir(fpath);

for i=1:length(dlist)
    if strcmp(dlist(i).name,'.') || strcmp(dlist(i).name,'..') continue; end

    if dlist(i).isdir
        fileList = getFileList(sprintf('%s%s%s',fpath,filesep,dlist(i).name),fileList);
        continue;
    end
    if strfind(dlist(i).name,'.dat')
        fileList{end+1} = sprintf('%s%s%s',fpath,filesep,dlist(i).name);
    end
end

%---------------------------------------
function taskDefs = loadTaskFile
taskDefs = [];
[fname,fpath] = uigetfile('*.ini','Select the INI file to use');
if fname == 0 return; end
fid = fopen([fpath,fname],'r');
if fid == -1 return; end
globalSource = [];
pos = 0;
err = 0;
while 1 && ~err
    line = fgetl(fid);
    if line == -1 break; end
    line = strtrim(line);
    if length(line) == 0 continue; end
    if strcmp(line(1),'%') continue; end

    line = tokLine(line);
    if isempty(line) continue; end
    if length(line{1}) == 0 continue; end

    if strcmp(lower(line{1}),'source')
        globalSource = line{2};
        continue;
    end
    if strcmp(lower(line{1}),'export')
        continue;
    end
    if strcmp(lower(line{1}), 'name')
        pos = pos+1;
        atEnd = false;
        taskDefs{pos} = initTask;
        taskDefs{pos}.name = line{2};
        
        while 1 && ~atEnd
            line = strtrim(fgetl(fid));
            if isempty(line) continue; end
            line = tokLine(line);
            switch lower(line{1})
                case 'states'
                    taskDefs{pos}.states = line(2:end);
                case 'parms'
                    taskDefs{pos}.parms = line(2:end);
                case 'amp'
                    taskDefs{pos}.amp.flag = 1;
                    taskDefs{pos}.amp.ch = str2num(line{2});
                case 'damp'
                    taskDefs{pos}.dAmp.flag = 1;
                    taskDefs{pos}.dAmp.ch = str2num(line{2});
                case 'vid'
                    taskDefs{pos}.vid.flag = 1;
                    taskDefs{pos}.vid.ch = str2num(line{2});
                    
                    taskDefs{pos}.vid.state = line{3};
                    for i=4:length(line)
                        taskDefs{pos}.vid.stateVal = [taskDefs{pos}.vid.stateVal, str2num(line{i})];
                    end
                case 'aud'
                    taskDefs{pos}.aud.flag = 1;
                    taskDefs{pos}.aud.ch = str2num(line{2});
                    taskDefs{pos}.aud.state = line{3};
                    for i=4:length(line)
                        taskDefs{pos}.aud.stateVal = [taskDefs{pos}.aud.stateVal, str2num(line{i})];
                    end
                case 'source'
                    taskDefs{pos}.source = line{2};
                case 'sigproc'
                    taskDefs{pos}.sigproc = line{2};
                case 'app'
                    taskDefs{pos}.app = line{2};
                case 'parm'
                    taskDefs{pos}.parm{end+1} = line{2};
                case 'skip'
                    taskDefs{pos}.skip = 1;
                case 'end'
                    atEnd = 1;
                case 'name'
                    err = 1;
                    atEnd = 1;
            end
        end
    end
end
fclose(fid);
% check tasks for duplicates
for i=1:length(taskDefs)
    for j=i+1:length(taskDefs)
        if strcmp(taskDefs{i}, taskDefs{j})
            waitfor(errordlg('Duplicate task names exist in ini file.'));
            taskDefs = [];
            return;
        end
    end
end


%---------------------------------
function lineToks = tokLine(str)
lineToks = [];
while ~isempty(str)
    [tok, str] = strtok(str);
    lineToks{end+1} = tok;
end

%----------------------------------
function newTask = initTask
newTask.name = '';
newTask.skip = 0;
newTask.states = [];
newTask.parms = [];
newTask.amp.flag = 0;
newTask.amp.ch = 0;
newTask.dAmp.flag = 0;
newTask.dAmp.ch = 0;
newTask.vid.flag = 0;
newTask.vid.ch = 0;
newTask.vid.state = '';
newTask.vid.stateVal = [];
newTask.aud.flag = 0;
newTask.aud.ch = 0;
newTask.aud.state = '';
newTask.aud.stateVal = [];
newTask.source = '';
newTask.sigproc = '';
newTask.app = '';
newTask.parm = [];

function analysisCode
%% analysis config

%a = [13 3 7 10];
%a = [11 1 5 8];
%a = [13 1 5 9];
%a = [14 2 6 10];
%a = [15 3 7 11];
a = [23 17 19 21];
SR = [512 1200 2400 4800];

for i=1:length(a)
    ampm(i) = mean(res{a(i)}.dAmp.vals);
    amps(i) = std(res{a(i)}.dAmp.vals);
    spm(i) = mean(res{a(i)}.sigProc);
    sps(i) = std(res{a(i)}.sigProc);
	jitterm(i) = mean(res{a(i)}.jitter);
    jitters(i) = std(res{a(i)}.jitter);
    vidm(i) = mean(res{a(i)}.vid.vals-res{a(i)}.vid.sigProc);
    vids(i) = std(res{a(i)}.vid.vals-res{a(i)}.vid.sigProc);
    SBlen(i) = 1000*res{a(i)}.SBS/res{a(i)}.fs;
end
figure; 
hold on;
pd = 4;
plot(SR(1:pd),SBlen(1:pd),'color',[0 0 0])
errorbar(SR(1:pd), ampm(1:pd), amps(1:pd));
errorbar(SR(1:pd), spm(1:pd), sps(1:pd),'r');
errorbar(SR(1:pd), jitterm(1:pd), jitters(1:pd),'g');
errorbar(SR(1:pd), vidm(1:pd), vids(1:pd));

%%
figure;
for i=1:4
    subplot(2,2,i);
    set(gca,'color',[0 0 0])
    hold on;
    plot([1, res{a(i)}.vid.stim(end)],[0 0]+SBlen(i),'color',[1 1 1])
    %plot(res{a(i)}.vid.stim, res{a(i)}.vid.vals);
    plot(res{a(i)}.vid.stim, res{a(i)}.vid.vals-res{a(i)}.vid.sigProc);
    plot(res{a(i)}.vid.stim,res{a(i)}.vid.sigProc,'r');
    plot(res{a(i)}.vid.stim,res{a(i)}.vid.jitter,'g');
    if ~isempty(res{a(i)}.aud)
        plot(res{a(i)}.aud.stim, res{a(i)}.aud.vals-res{a(i)}.aud.sigProc,'color',[1.0000    0.6941    0.3922]);
    end
    b = find(res{a(i)}.vid.stim < length(res{a(i)}.amp.ts));
    plot(res{a(i)}.dAmp.ts/res{a(i)}.SBS, res{a(i)}.dAmp.vals,'y');
    YL = ylim;
    set(gca,'ylim',[0 YL(end)]);
    
    title(num2str(SR(i)));
    if i==1
        xlabel('Trial');
        ylabel('Timing (ms)');
    end
end
%%

%% print tasks
for i=1:length(res) 
    if ~isfield(res{i},'fs')
        e ='*';
    else
        e = '';
    end
    fprintf('%d %s%s\n', i, e,res{i}.name); 
end  