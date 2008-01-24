% Analyzes data from the BCI2000 oddball paradigm
%
% (1) data has to be converted to a Matlab .mat file using BCI2ASCII
%     IMPORTANT: Set "Increment trial # if state" to "IconVisible" and "1"
%
% (2) call this function
%     syntax: [res1, res2, ressqch] = oddball(subject, samplefreq, channel, triallength, plotthis)
%
%             [input]
%             subject    ... data file name (with .mat extension)
%             samplefreq ... the data's sampling rate
%             channel    ... channel of interest (e.g., Cz or Pz)
%             triallength .. how long do we want the waveforms ?
%             plotthis   ... if 1, plots results; if 0, does not plot
%
%             [output]
%             res1 ... average waveform for 'standard' condition
%             res2 ... average waveform for 'oddball' condition
%             rsq  ... r-squared between standard and oddball condition for each point in time
%
% (C) Gerwin Schalk 2002
%     Wadsworth Center, NYSDOH
function [res1, res2, ressqch] = oddball(subject, samplefreq, channel, triallength, plotthis)

%channel=51;
%samplefreq=256;
%subject='D:\\BCI2000DATA\\gs001\\gs.mat'; 
%triallength=144;

% load session
loadcmd=sprintf('load %s', subject);
eval(loadcmd);

avgdata1=[];
avgdata2=[];
trials=unique(trialnr);
for cur_trial=min(trials):max(trials)-2
 % fprintf(1, 'Trial: %03d/%03d\n', cur_trial, max(trials));
 % get the indeces of the samples of the right trial and determine which target it was
 trialidx=find(trialnr == cur_trial);
 cur_targetcode=max(IconNumber(trialidx));
 trialdata=signal(min(trialidx):min(trialidx+triallength)-1, :);
 if (cur_targetcode == 1)
    avgdata1=cat(3, avgdata1, single(trialdata));
 end
 if (cur_targetcode == 2)
    avgdata2=cat(3, avgdata2, single(trialdata));
 end
end

% calculate average trials for each condition and each channel
res1=mean(avgdata1, 3);
res2=mean(avgdata2, 3);

% calculate rsqu for each channel and each sample
ressqall = calc_rsqu(avgdata1, avgdata2);
ressqch=ressqall(:, channel);

% plot the results, if so desired
if (plotthis == 1)  
   timems=[1:triallength]'/samplefreq*1000;
   figure(1);
   clf;
   subplot(2, 1, 1);
   plot(timems, res1(:, channel), 'r');
   hold on;
   plot(timems, res2(:, channel), 'b');
   title('Time Course and Signal for Two Conditions');
   subplot(2, 1, 2);
   plot(timems, ressqch);
   title('R2 Between Two Conditions');
end

