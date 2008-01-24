% Analyzes data from the BCI2000 P3 spelling paradigm
%
% (1) data has to be converted to a Matlab .mat file using BCI2ASCII
%     IMPORTANT: Set "Increment trial # if state" to "Flashing" and "1"
%
% (2) call this function
%     syntax: [res1ch, res2ch, ressqch] = p3(subject, samplefreq, channels, triallength, rorrsqu, 
%                                            plotthis, topotime, eloc_file, moviefilename)
%
%             [input]
%             subject    ... data file name (with .mat extension)
%             samplefreq ... the data's sampling rate (e.g., 240)
%             filters    ... hp and lp frequencies in Hz for analog
%                            filtering (e.g., [0.1 30]) or [-1 -1] if no filtering requested
%             channels   ... channels of interest (e.g., [15 11])
%             triallength .. length of the displayed waveform in ms
%             rorrsq     ... if 1, calculates r^2 values; if 0, calculates r
%             plotthis   ... if 1, plots results; if 0, does not plot
%             topotimes_ms . a list of times to create topographies (if [], no topography will be plotted) (times in ms)
%             topogrid   ... the layout for the topographies (e.g., [4 3] for a 4x3 matrix of topographies)
%             baseline_ms .. duration of the baseline period in ms (e.g., [0 100]) or [] if no baseline correction requested
%             eloc_file  ... file that contains the electrode positions (e.g., eloc64.txt, eloc16.txt)
%             moviefilename. if not '', specifies the filename of the avi movie file
%
%             [output]
%             res1ch       ... average waveform for 'standard' condition for desired channel
%             res2ch       ... average waveform for 'oddball' condition for desired channel
%             ressqch      ... r-squared between standard and oddball condition for each point in time for desired channel
%             stimulusdata ... average waveforms for each stimulus
%
% (C) Gerwin Schalk 2002-04
%     Wadsworth Center, NYSDOH
%
% V1.00 - 09/2002 - first version
% V1.10 -           can now plot a number of topographies
% V1.30 - 10/2004 - speed increased; can now also plot a number of traces
% V1.40 - 12/2004 - can now perform baseline correction and analog filtering

function [res1ch, res2ch, ressqch, stimulusdata] = p3(subject, samplefreq, filters, channels, triallength, rorrsqu, plotthis, topotimes_ms, topogrid, baseline_ms, eloc_file, moviefilename)

topotimes_samples=round(topotimes_ms*samplefreq/1000);				% convert from ms into samples
triallength=round(triallength*samplefreq/1000);                     % convert from ms into samples

fprintf(1, 'BCI2000 P3 Analysis Routine V1.40\n');
fprintf(1, '(C) 2002-04 Gerwin Schalk\n');
fprintf(1, '=================================\n');

res1ch=[];
res2ch=[];
ressqch=[];
stimulusdata=[];

% sanity checks
if ((length(baseline_ms) ~= 2) & (isempty(baseline_ms) == 0))
   fprintf(1, 'baseline correction option has to specify either [] or two numbers in ms (e.g., [0 100]). Aborting.\n');
   return;
end
if (isempty(baseline_ms) == 0)
   if (baseline_ms(1) >= baseline_ms(2))
      fprintf(1, 'Second number in baseline definition has to be larger than the first number. Aborting.\n');
      return;
   end
end

baseline_samples=round(baseline_ms*samplefreq/1000);				% convert from ms into samples
% if we specify [0 100], we have to convert sample 0 to sample 1
if (isempty(baseline_samples) == 0)
   baseline_samples(find(baseline_samples < 1))=1;
end

% load session
fprintf(1, 'Loading data file\n');
loadcmd=sprintf('load %s', subject);
eval(loadcmd);

% perform analog filtering before we do the analysis
hpfreq=filters(1);
lpfreq=filters(2);
if (hpfreq > 0)   % only perform the filtering if we want to
   fprintf(1, 'bandpass filtering signal\n');
   if (lpfreq > 0)
      [b, a]=butter(3, [(hpfreq/samplefreq)*2 (lpfreq/samplefreq)*2]);
   else
      [b, a]=butter(3, (hpfreq/samplefreq)*2);
   end
   for ch=1:size(signal, 2)
    signal(:, ch)=filter(b, a, signal(:, ch));
   end
end

% determine the number of target and non-target stimuli
% so that we can pre-allocate the avgdata arrays
count_stimtype0=0;
count_stimtype1=0;
trials=unique(trialnr);
fprintf(1, 'Determining number of target/non-target stimuli: ');
for cur_trial=min(trials)+1:max(trials)-1
 trialidx=find(trialnr == cur_trial);
 cur_stimulustype=max(StimulusType(trialidx));
 if (cur_stimulustype == 0)
    count_stimtype0=count_stimtype0+1;
 else
    count_stimtype1=count_stimtype1+1;
 end
end
fprintf(1, '%d/%d\n', count_stimtype0, count_stimtype1);

num_chans=size(signal, 2);
avgdata1=zeros(triallength, num_chans, count_stimtype0);
avgdata2=zeros(triallength, num_chans, count_stimtype1);
max_stimuluscode=max(StimulusCode);
stimulusdata=zeros(max_stimuluscode, triallength);
stimuluscount=zeros(max_stimuluscode);
count0=1;
count1=1;
fprintf(1, 'Processing all trials (i.e., stimuli)\n');
for cur_trial=min(trials)+1:max(trials)-1
 if (mod(cur_trial+1, 50) == 0)
    fprintf(1, '%03d ', cur_trial+1);
    if (mod(cur_trial+1, 300) == 0)
       fprintf(1, '* /%d\r', max(trials));
    end
 end
 % get the indeces of the samples of the right trial
 trialidx=find(trialnr == cur_trial);
 % get the data for these samples
 %trialdata=signal(min(trialidx):min(trialidx)+triallength-1, cat(2, [1:16], [19:64]));
 trialdata=signal(min(trialidx):min(trialidx)+triallength-1, :);
 % apply baseline correction, if requested
 if (isempty(baseline_samples) == 0)
    baseline=mean(trialdata([min(baseline_samples):max(baseline_samples)], :), 1);
    for ch=1:size(trialdata, 2)
     trialdata(:, ch)=trialdata(:, ch)-baseline(ch);
    end
 end
 % apply a 'spatial' filter, i.e., subtract the average of 5 electrodes
 % reference=mean((trialdata(:, 1)+trialdata(:, 2)+trialdata(:, 10)+trialdata(:, 16)+trialdata(:, 9))/5);
 % trialdata=trialdata-reference;
 cur_stimulustype=max(StimulusType(trialidx));
 cur_stimuluscode=max(StimulusCode(trialidx));
 stimulusdata(cur_stimuluscode, :)=stimulusdata(cur_stimuluscode, :)+trialdata(:, channels(1))';
 stimuluscount(cur_stimuluscode)=stimuluscount(cur_stimuluscode)+1;
 if (cur_stimulustype == 0)
    avgdata1(:, :, count0, :)=double(trialdata);
    count0=count0+1;
 else
    avgdata2(:, :, count1, :)=double(trialdata);
    count1=count1+1;
 end
end % session

fprintf(1, '\r');

fprintf(1, 'Calculating statistics\n');

% calculate average responses for each of the stimuli
for stim=1:max_stimuluscode
 stimulusdata(stim, :)=stimulusdata(stim, :)/stimuluscount(stim);
end

% calculate average trials for each condition and each channel
res1=mean(avgdata1, 3);
res2=mean(avgdata2, 3);
res1ch=res1(:, channels(1));
res2ch=res2(:, channels(1));

% calculate rsqu for each channel and each sample between up and down target
ressq = calc_rsqu(avgdata1, avgdata2, rorrsqu);
ressqch = ressq(:, channels(1));

timems=[1:triallength]/samplefreq*1000;

fprintf(1, 'Plotting results\n');

if (plotthis == 1)    
   figure(1);
   clf;
   % determine the axis scaling (should be same for all channels)
   min_res=1e5;
   max_res=-1e5;
   max_r2=-1;
   for cur_ch=1:length(channels)
    cur_min=min([min(res1(:, channels(cur_ch))) min(res2(:, channels(cur_ch)))]);
    cur_max=max([max(res1(:, channels(cur_ch))) max(res2(:, channels(cur_ch)))]);
    cur_maxr2=max([max(ressq(:, channels(cur_ch))) max(ressq(:, channels(cur_ch)))]);
    if (cur_min < min_res)
       min_res=cur_min;
    end
    if (cur_max > max_res)
       max_res=cur_max;
    end
    if (cur_maxr2 > max_r2)
       max_r2=cur_maxr2;
    end
   end
   for cur_ch=1:length(channels)
    subplot(2, length(channels), cur_ch);
    plot(timems, res1(:, channels(cur_ch)), 'r');
    hold on;
    plot(timems, res2(:, channels(cur_ch)), 'b:');
    titletxt=sprintf('Time Course of P300 Amplitude (ch%02d)', channels(cur_ch));
    title(titletxt);
    legend('standard', 'oddball');
    axis([min(timems) max(timems) min_res max_res]);
    subplot(2, length(channels), length(channels)+cur_ch);
    plot(timems, ressq(:, channels(cur_ch)));
    titletxt=sprintf('Time Course of P300 r2 (ch%02d)', channels(cur_ch));
    title(titletxt);
    xlabel('Time After Stimulus (ms)');	%Added by Eric 8/20/02
    ylabel('Prop r2');						%Added by Eric 8/20/02
    axis([min(timems) max(timems) 0 max_r2]);
   end
   figure(3);
   surf(timems, [1:size(ressq, 2)], ressq');
   axis([min(timems) max(timems) 1 size(ressq, 2)]);
   view(2);
   shading interp;
   colorbar;
   title('Time Course of P300 r2 Across Channels');
   xlabel('Time After Stimulus (ms)');
   ylabel('Channel Number');
   % plot individual responses
   figure(4);
   clf;
   displaymin=min(min(stimulusdata));
   displaymax=max(max(stimulusdata));
   for stim=1:max_stimuluscode
    subplot(max_stimuluscode, 1, stim);
    plot(timems, stimulusdata(stim, :));
    axis([min(timems) max(timems) displaymin displaymax]);
    labeltxt=sprintf('Stim %d', stim);
    ylabel(labeltxt);
    if (stim == 1)
       titletxt=sprintf('ERP Responses To Each of 12 Stimuli (ch%02d)', channels(1));
       title(titletxt);
    end
   end
   % plot averaged responses for each of the 36 grid positions
   if (max_stimuluscode == 12)  % then assume the speller
      figure(5);
      clf;
      for x=1:6
       for y=1:6
        h=subplot(6, 6, x+(y-1)*6);
        avgresponse=mean(stimulusdata([x (y-1)+7], :), 1);   % targetID is calculated wrong !!!
        plot(timems, avgresponse);
        axis([min(timems) max(timems) displaymin displaymax]);
        %set(h, 'XGrid', 'on');
        set(h, 'XTick', []);
        set(h, 'XTickLabel', []);
        %set(h, 'YGrid', 'on');
        set(h, 'YTick', []);
        set(h, 'YTickLabel', []);
       end
      end
   end
end

% if we want to create a topography
num_topos=max(size(topotimes_samples));
if (num_topos > topogrid(1)*topogrid(2))
   fprintf(1, 'Warning: Number of topographies to be plotted exceeds size of topo matrix');
   num_topos=topogrid(1)*topogrid(2);
end
if (num_topos > 0)
   figure(2);
   clf;
   displaymin=1000;
   displaymax=-1000;
   for cur_topo=1:num_topos
     if (min(ressq(topotimes_samples(cur_topo), :)) < displaymin)
        displaymin=min(ressq(topotimes_samples(cur_topo), :));
     end
     if (max(ressq(topotimes_samples(cur_topo), :)) > displaymax)
        displaymax=max(ressq(topotimes_samples(cur_topo), :));
     end
   end   % plot all topographies
   for cur_topo=1:num_topos
     subplot(topogrid(2), topogrid(1), cur_topo);
     data2plot=ressq(topotimes_samples(cur_topo), :);
     topoplot(data2plot, eloc_file, 'maplimits', [displaymin, displaymax], 'style', 'straight', 'gridscale', 100);
     titletxt=sprintf('r^2 at %.1f ms', topotimes_ms(cur_topo));
     title(titletxt); 
     if (cur_topo == num_topos)
        colorbar;
     end
   end
end % if any topography


% if we want, generate a movie
if (~isempty(moviefilename))
   FPS=samplefreq/10;
   timestretch=10;
   % now, create the movie
   make_movie(moviefilename,  ressq, size(avgdata1, 1), samplefreq, FPS, timestretch, min(min(ressq)), max(max(ressq)), '', eloc_file);
end

