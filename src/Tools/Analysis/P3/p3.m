% Analyzes data from the BCI2000 P3 spelling paradigm
%
% (1) data has to be converted to a Matlab .mat file using BCI2ASCII
%     IMPORTANT: Set "Increment trial # if state" to "Flashing" and "1"
%
% (2) call this function
%     syntax: [res1ch, res2ch, ressqch] = p3(subject, samplefreq, channel, triallength,
%                                            plotthis, topotime, eloc_file, moviefilename)
%
%             [input]
%             subject    ... data file name (with .mat extension)
%             samplefreq ... the data's sampling rate (e.g., 240)
%             channel    ... channel of interest (e.g., 15 or 11)
%             triallength .. length of the displayed waveform in ms
%             plotthis   ... if 1, plots results; if 0, does not plot
%             topotime   ... if > 0, does topography at time 'topotime' (time in ms)
%             eloc_file  ... file that contains the electrode positions (e.g., eloc64.txt, eloc16.txt)
%             moviefilename. if not '', specifies the filename of the avi movie file
%
%             [output]
%             res1ch       ... average waveform for 'standard' condition for desired channel
%             res2ch       ... average waveform for 'oddball' condition for desired channel
%             ressqch      ... r-squared between standard and oddball condition for each point in time for desired channel
%             stimulusdata ... average waveforms for each stimulus
%
% (C) Gerwin Schalk 2002
%     Wadsworth Center, NYSDOH
%
function [res1ch, res2ch, ressqch, stimulusdata] = p3(subject, samplefreq, channel, triallength, plotthis, topotime, eloc_file, moviefilename)

%channel=11;
%samplefreq=240;

%subject='D:\\BCI2000DATA\\tv\\tv002\\tv1_8.mat'; 
%session=1;
%triallength=144;
%moviefilename='d:\TV001P3.avi';
%moviefilename=[];

%topotime=81;
topotime=round(topotime*samplefreq/1000);				% convert from ms into samples
triallength=round(triallength*samplefreq/1000);    % convert from ms into samples

% load session
loadcmd=sprintf('load %s', subject);
eval(loadcmd);

avgdata1=[];
avgdata2=[];
trials=unique(trialnr);
max_stimuluscode=max(StimulusCode);
stimulusdata=zeros(max_stimuluscode, triallength);
stimuluscount=zeros(max_stimuluscode);
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
 % apply a 'spatial' filter, i.e., subtract the average of 5 electrodes
 % reference=mean((trialdata(:, 1)+trialdata(:, 2)+trialdata(:, 10)+trialdata(:, 16)+trialdata(:, 9))/5);
 % trialdata=trialdata-reference;
 cur_stimulustype=max(StimulusType(trialidx));
 cur_stimuluscode=max(StimulusCode(trialidx));
 stimulusdata(cur_stimuluscode, :)=stimulusdata(cur_stimuluscode, :)+trialdata(:, channel)';
 stimuluscount(cur_stimuluscode)=stimuluscount(cur_stimuluscode)+1;
 if (cur_stimulustype == 0)
    avgdata1=cat(3, avgdata1, double(trialdata));
 end
 if (cur_stimulustype == 1)
    avgdata2=cat(3, avgdata2, double(trialdata));
 end
end % session

fprintf(1, '\r');

% calculate average responses for each of the stimuli
for stim=1:max_stimuluscode
 stimulusdata(stim, :)=stimulusdata(stim, :)/stimuluscount(stim);
end

% calculate average trials for each condition and each channel
res1=mean(avgdata1, 3);
res2=mean(avgdata2, 3);
res1ch=res1(:, channel);
res2ch=res2(:, channel);

% calculate rsqu for each channel and each sample between up and down target
ressq = calc_rsqu(avgdata1, avgdata2);
ressqch = ressq(:, channel);

timems=[1:triallength]/samplefreq*1000;

if (plotthis == 1)    
   figure(1);
   clf;
   subplot(2, 1, 1);
   plot(timems, res1(:, channel), 'r');
   hold on;
   plot(timems, res2(:, channel), 'b:');
   title('Time Course of P300 Amplitude');
   legend('standard', 'oddball');
   subplot(2, 1, 2);
   plot(timems, ressq(:, channel));
   title('Time Course of P300 r2');
   xlabel('Time After Stimulus (ms)');	%Added by Eric 8/20/02
   ylabel('Prop r2');						%Added by Eric 8/20/02
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
       title('ERP Responses To Each of 12 Stimuli');
    end
   end
   % plot averaged responses for each of the 36 grid positions
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

% if we want to create a topography
if (topotime > 0)
   figure(2);
   data2plot=ressq(topotime, :);
   topoplot(data2plot, eloc_file, 'maplimits', [min(min(data2plot)), max(max(data2plot))], 'style', 'straight');
   colorbar;
end

% if we want, generate a movie
if (~isempty(moviefilename))
   FPS=samplefreq/10;
   timestretch=10;
   % now, create the movie
   make_movie(moviefilename,  ressq, size(avgdata1, 1), samplefreq, FPS, timestretch, min(min(ressq)), max(max(ressq)), '', eloc_file);
end

