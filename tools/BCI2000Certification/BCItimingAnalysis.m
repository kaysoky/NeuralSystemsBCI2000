function res = BCItimingAnalysis(res)
if exist('res')
    analysisFuncs(res);
    return;
end
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
%analysisFuncs(res);
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
res.nChs = size(signal,2);
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
    tmpsig = reshape(signal(:,thisTask.dAmp.ch+1),[SBS length(signal)/SBS]);
    [q,r] = max(tmpsig);
    ts = 1:length(q);
    a = find(q == 0);    
    q(a) = [];
    r(a) = [];
    ts(a) = [];
%     a = find(signal(:,thisTask.dAmp.ch+1) == max(signal(:,thisTask.dAmp.ch+1)));
%     a(find(a <= ignoreDur)) = [];
%     b = find(diff(a) < SBS/3)+1;
%     a(b) = [];
    res.dAmp.vals = 1000*(r-1)/fs;
    res.dAmp.ts = ts;
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
    res.vid.stim = [];
    res.vid.sigProc = [];
    res.vid.jitter = [];
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
    res.aud.stim = [];
    res.aud.sigProc = [];
    res.aud.jitter = [];
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
function lineToks = tokLine(str,del)
if ~exist('del') del = ' '; end
lineToks = [];
while ~isempty(str)
    [tok, str] = strtok(str, del);
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
newTask.nChs = 0;
%%
function analysisFuncs(res)

tasksAll = struct;
for i=1:length(res)
    tmp = tokLine(res{i}.name, '_');
    tmpTaskName = sprintf('%s%s%s', tmp{1}, tmp{end-1}, tmp{end});
    tmpBaseName = tmp{1};
    if ~isfield(tasksAll, tmpTaskName)        
        newTask.fs = [];
        newTask.idx = [];
        newTask.SBS = [];
        newTask.baseName = tmp{1};
        newTask.nChs = res{i}.nChs;
        newTask.dt = tmp{end};
       
        tasksAll = setfield(tasksAll, tmpTaskName, newTask);
    end
    tmpTask = getfield(tasksAll, tmpTaskName);
    tmpTask.fs = [tmpTask.fs, res{i}.fs];
    tmpTask.idx = [tmpTask.idx, i];
    tmpTask.SBS = [tmpTask.SBS, res{i}.SBS];
    tasksAll = setfield(tasksAll, tmpTaskName, tmpTask);
    clear newTask tmpTask tmpTaskName tmp;
end

taskfields = fieldnames(tasksAll);
for i=1:length(taskfields)
    tmpTask = getfield(tasksAll, taskfields{i});
    [tmpfs, idx] = sort(tmpTask.fs);
    tmpTask.fs = tmpfs;
    tmpTask.idx = tmpTask.idx(idx);
    tmpTask.SBS = tmpTask.SBS(idx);
    tasksAll = setfield(tasksAll, taskfields{i}, tmpTask);
end

taskChGrp = struct;
totalSamplesGrp = struct;
for i=1:length(taskfields)
    tmpTask = getfield(tasksAll, taskfields{i});
    if ~isfield(taskChGrp, tmpTask.baseName)
        taskChGrp = setfield(taskChGrp, tmpTask.baseName,[]);
        newTask.idx = [];
        newTask.nSamples = [];
        newTask.SBS = [];
        totalSamplesGrp = setfield(totalSamplesGrp, tmpTask.baseName, newTask);
    end
    tmpField = getfield(taskChGrp, tmpTask.baseName);
    tmpField{end+1} = tmpTask;
    taskChGrp = setfield(taskChGrp, tmpTask.baseName, tmpField);
    
    tmpField = getfield(totalSamplesGrp, tmpTask.baseName);
    tmpField.idx = [tmpField.idx, tmpTask.idx];
    tmpField.nSamples = [tmpField.nSamples, tmpTask.SBS*tmpTask.nChs];
    tmpField.SBS = max([tmpField.SBS, tmpTask.SBS*1000./tmpTask.fs]);
    totalSamplesGrp = setfield(totalSamplesGrp, tmpTask.baseName, tmpField);
end
taskChFields = fieldnames(taskChGrp);
for i=1:length(taskChFields)
    tmpField = getfield(taskChGrp, taskChFields{i});
    chTmp = [];
    for c=1:length(tmpField)
        chTmp(c) = tmpField{c}.nChs;
    end
    [q,idx] = sort(chTmp);
    for c=1:length(idx)
        newField{c} = tmpField{idx(c)};
    end
    taskChGrp = setfield(taskChGrp, taskChFields{i}, newField);
end

for i=1:length(taskChFields)
    newField = struct;
    tmpField = getfield(totalSamplesGrp, taskChFields{i});
    [q,idx] = sort(tmpField.nSamples);
    newField.nSamples = tmpField.nSamples(idx);
    newField.idx = tmpField.idx(idx);
    newField.SBS = tmpField.SBS;
    totalSamplesGrp = setfield(totalSamplesGrp, taskChFields{i}, newField);
end
clear taskfields tmpTask;
%%
taskfields = fieldnames(tasksAll);
outdir = uigetdir;
if outdir == 0 return; end
LW = 1;
for i=1:length(taskfields)
    figtmp = figure('name', taskfields{i},'position',[0 0 800 800],'color',[1 1 1]);
    tmptask = getfield(tasksAll, taskfields{i});
    for k=1:length(tmptask.idx)
        subplot(2,2,k);
        set(gca,'fontsize',16);
        
        %set(gca,'color',[0 0 0])
        hold on;
        p = tmptask.idx(k);
        SBS = res{p}.SBS;
        fs = res{p}.fs;
        SBlen = 1000*SBS/fs;
        if mean(res{p}.jitter) >= SBlen continue; end
        
        plot([0, max(res{p}.vid.stim(end),res{p}.dAmp.ts(end))*SBS/fs],[0 0]+SBlen,':','color',[0 0 0])
        %plot(res{p}.vid.stim, res{p}.vid.vals);
        plot(res{p}.vid.stim*SBS/fs, res{p}.vid.vals-res{p}.vid.sigProc,'linewidth',LW);
        plot(res{p}.vid.stim*SBS/fs,res{p}.vid.sigProc,'r','linewidth',LW);
        plot(res{p}.vid.stim*SBS/fs,res{p}.vid.jitter,'g','linewidth',LW);
        if ~isempty(res{p}.aud)
            plot(res{p}.aud.stim*SBS/fs, res{p}.aud.vals-res{p}.aud.sigProc,'color',[1 0 1], 'linewidth', LW);%[1.0000    0.6941    0.3922],'linewidth',2);
        end
        b = find(res{p}.vid.stim < length(res{p}.amp.ts));
        plot(res{p}.dAmp.ts*SBS/fs, res{p}.dAmp.vals,'color',[0 0 0],'linewidth',LW);
        axis tight;
        YL = ylim;
        XL = xlim;
        set(gca,'ylim',[0 YL(end)*1.1]);

        title(sprintf('%s Hz', num2str(res{p}.fs)));
        xlabel('Time (s)');
        if k==1
            
            ylabel('Delay (ms)');
        end        
    end
    saveas(figtmp, sprintf('%s%s%s.fig', outdir, filesep, taskfields{i}));
    saveas(figtmp, sprintf('%s%s%s.png', outdir, filesep, taskfields{i}));
    saveas(figtmp, sprintf('%s%s%s.eps', outdir, filesep, taskfields{i}),'epsc2');
    close(figtmp);
end

%%
taskChFields = fieldnames(taskChGrp);
for j=1:length(taskChFields)
    tmpField = getfield(taskChGrp, taskChFields{j});
    figtmp = figure('name', taskChFields{j},'position',[0 0 800 800],'color',[1 1 1]);
    sysLatAll = [];
    
    for k=1:length(tmpField)
        a = tmpField{k}.idx;
        subplot(2,2,k);
        set(gca,'fontsize',16);
        hold on;
        clear ampm amps spm sps jitterm jitters vidm vids SBlen audm auds SR sysm
        audm = [];
        auds = [];
        p=1;
        for i=1:length(a)
            if mean(res{a(i)}.jitter) >= 1000*res{a(i)}.SBS/res{a(i)}.fs continue; end
            SR(p) = res{a(i)}.fs;
            ampm(p) = mean(res{a(i)}.dAmp.vals);
            amps(p) = std(res{a(i)}.dAmp.vals);
            spm(p) = mean(res{a(i)}.sigProc);
            sps(p) = std(res{a(i)}.sigProc);
            jitterm(p) = mean(res{a(i)}.jitter);
            jitters(p) = std(res{a(i)}.jitter);
            vidm(p) = mean(res{a(i)}.vid.vals-res{a(i)}.vid.sigProc-ampm(p));
            vids(p) = std(res{a(i)}.vid.vals-res{a(i)}.vid.sigProc-ampm(p));
            if ~isempty(res{a(i)}.aud)
                audm(p) = mean(res{a(i)}.aud.vals-res{a(i)}.aud.sigProc);
                auds(p) = std(res{a(i)}.aud.vals-res{a(i)}.aud.sigProc);
            end
            sysm(p) = mean(res{a(i)}.vid.vals);
            syss(p) = std(res{a(i)}.vid.vals);
            SBlen(p) = 1000*res{a(i)}.SBS/res{a(i)}.fs;
            p=p+1;
        end
        
        plot(SR,SBlen,'color',[0 0 0])
        errorbar(SR, ampm, amps,'linewidth',LW);
        errorbar(SR, spm, sps,'r','linewidth',LW);
        errorbar(SR, jitterm, jitters,'g','linewidth',LW);

        errorbar(SR, vidm, vids,'color',[.5 .5 .5],'linewidth',LW);
        if ~isempty(audm)
            errorbar(SR, audm, auds,'color',[1.0000    0.6941    0.3922],'linewidth',LW);
        end        
        axis tight;
        YL = ylim;
        XL = xlim;
        set(gca,'ylim',[0 YL(end)*1.1]);

        title(sprintf('%s Chs', num2str(tmpField{k}.nChs)));
        
        if k==1        
            xlabel('Sample Rate');
            ylabel('Delay (ms)');
        end   
        
        sysLatAll{k} = [sysm; SR];
    end
    subplot(2,2,4);
    hold on;
    set(gca,'fontsize',16);
    for i=1:length(sysLatAll)
        plot(sysLatAll{i}(2,:), sysLatAll{i}(1,:),'color',[0 0 0],'linewidth',LW);
    end
     axis tight;
    YL = ylim;
    XL = xlim;
    set(gca,'ylim',[0 100*1.1], 'xlim',[0 XL(end)*1.1]);
    title(sprintf('System Latency'));

    xlabel('Sample Rate');
    
    saveas(figtmp, sprintf('%s%s%s.fig', outdir, filesep, taskChFields{j}));
    saveas(figtmp, sprintf('%s%s%s.png', outdir, filesep, taskChFields{j}));
    saveas(figtmp, sprintf('%s%s%s.eps', outdir, filesep, taskChFields{j}),'epsc2');
    close(figtmp);
%     clear ampm amps spm sps jitterm jitters vidm vids SBlen audm auds SR nSamples
%     audm = [];
%     auds = [];
%     tmpField = getfield(totalSamplesGrp, taskChFields{j});
%     a = tmpField.idx;
%     p=1;
%     for i=1:length(a)
%         if mean(res{a(i)}.jitter) >= 1000*res{a(i)}.SBS/res{a(i)}.fs continue; end
%         nSamples(p) = tmpField.nSamples(i);
%         ampm(p) = mean(res{a(i)}.dAmp.vals);
%         amps(p) = std(res{a(i)}.dAmp.vals);
%         spm(p) = mean(res{a(i)}.sigProc);
%         sps(p) = std(res{a(i)}.sigProc);
%         jitterm(p) = mean(res{a(i)}.jitter);
%         jitters(p) = std(res{a(i)}.jitter);
%         vidm(p) = mean(res{a(i)}.vid.vals-res{a(i)}.vid.sigProc);
%         vids(p) = std(res{a(i)}.vid.vals-res{a(i)}.vid.sigProc);
%         if ~isempty(res{a(i)}.aud)
%             audm(p) = mean(res{a(i)}.aud.vals-res{a(i)}.aud.sigProc);
%             auds(p) = std(res{a(i)}.aud.vals-res{a(i)}.aud.sigProc);
%         end
%         p=p+1;
%     end
%     plot(nSamples, spm,'r');
%     plot(nSamples, ampm,'color',[0 0 0]);
%     plot(nSamples, vidm,'color',[0 0 1]);
%     plot(nSamples, jitterm,'g');
   
end

save(sprintf('%s%sresults.mat', outdir, filesep), 'res');
return;
%%
figure;
for i=1:length(a)
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
    
clear ampm amps spm sps jitterm jitters vidm vids SBlen audm auds
audm = [];
auds = [];
for i=1:length(a)
    ampm(i) = mean(res{a(i)}.dAmp.vals);
    amps(i) = std(res{a(i)}.dAmp.vals);
    spm(i) = mean(res{a(i)}.sigProc);
    sps(i) = std(res{a(i)}.sigProc);
	jitterm(i) = mean(res{a(i)}.jitter);
    jitters(i) = std(res{a(i)}.jitter);
    vidm(i) = mean(res{a(i)}.vid.vals-res{a(i)}.vid.sigProc);
    vids(i) = std(res{a(i)}.vid.vals-res{a(i)}.vid.sigProc);
    if ~isempty(res{a(i)}.aud)
        audm(i) = mean(res{a(i)}.aud.vals-res{a(i)}.aud.sigProc);
        auds(i) = std(res{a(i)}.aud.vals-res{a(i)}.aud.sigProc);
    end
    SBlen(i) = 1000*res{a(i)}.SBS/res{a(i)}.fs;
end
figure; 
hold on;
pd = 4;
plot(SR,SBlen,'color',[0 0 0])
errorbar(SR, ampm, amps);
errorbar(SR, spm, sps,'r');
errorbar(SR, jitterm, jitters,'g');

errorbar(SR, vidm, vids,'color',[.5 .5 .5]);
if ~isempty(audm)
    errorbar(SR, audm, auds,'color',[1.0000    0.6941    0.3922]);
end
%%
% setup figure
set(gca,'fontsize',16);

%% print tasks
for i=1:length(res) 
    if ~isfield(res{i},'fs')
        e ='*';
    else
        e = '';
    end
    fprintf('%d %s%s\n', i, e,res{i}.name); 
end  