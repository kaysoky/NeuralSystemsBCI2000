%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% $Id: UGenericSignal.m 2007-111-26 12:31:37EST fialkoff $ 
%% 
%% File: UGenericSignal.m 
%% 
%% Author: Joshua Fialkoff <fialkj@rpi.edu>, Gerwin Schalk <schalk@wadsworth.org>
%%
%% Description: This function performs a frequency-domain analysis of the
%% specified data.
%%
%% (C) 2000-2008, BCI2000 Project
%% http:%%www.bci2000.org 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function [res1, res2, ressq, freq_bins] = freqAnalysis(analysisParams, ...
  signal, trialnr, params, states, parms, spectral_size, ..., 
  spectral_stepping, verbose, errorOverride, minTrialsPerCondWarn, ...
  minTrialsPerCondErr)

funcName = 'freqAnalysis';

cond1str = analysisParams.targetConditions{1};
cond1txt = analysisParams.conditionLabels{1};
condition1idxstr=sprintf('condition1idx=find((trialnr == cur_trial) & %s);', cond1str);

if length(analysisParams.targetConditions) == 2
  cond2str = analysisParams.targetConditions{2};
  cond2txt = analysisParams.conditionLabels{2};
  condition2idxstr=sprintf('condition2idx=find((trialnr == cur_trial) & %s);', cond2str);
end

eloc_file = analysisParams.montageFile;

samplefreq=str2num(params.SamplingRate.Value{1});



avgdata1=[];
avgdata2=[];
trials=unique(trialnr);
start=parms(2);
stop=parms(3);
binwidth=parms(4);
num_channels=size(signal, 2);

spectral_bins=round((stop-start)/binwidth)+1;       % for AR

% determine the size of the arrays to speed up calculations
countall=0;
countcond1=0;
countcond2=0;
if verbose
  fprintf(1, 'Determining array sizes ...\n');
end
for cur_trial=min(trials):max(trials)-1
 if verbose && (mod(cur_trial+1, 50) == 0)
    fprintf(1, '%03d ', cur_trial+1);
    if (mod(cur_trial+1, 150) == 0)
       fprintf(1, '* /%d\r', max(trials));
    end
 end
 eval(condition1idxstr);
 if (isempty(condition1idx) == 0)
    datalength=length(condition1idx);
    cur_start=1;
    count=0;
    while (cur_start+spectral_size <= datalength)
     cur_start=cur_start+spectral_stepping;
     count=count+1;
     countall=countall+1;
    end
    if (count > 0)
       countcond1=countcond1+1;
    end
 end

 if length(analysisParams.targetConditions) == 2
   eval(condition2idxstr);
   if (isempty(condition2idx) == 0)
      datalength=length(condition2idx);
      cur_start=1;
      count=0;
      while (cur_start+spectral_size <= datalength)
       cur_start=cur_start+spectral_stepping;
       count=count+1;
       countall=countall+1;
      end
      if (count > 0)
         countcond2=countcond2+1;
      end
   end
  end
end

if verbose
  fprintf(1, '\r');
end

% pre-allocate matrices to speed up computation
avgdata1=zeros(spectral_bins, num_channels, countcond1);
if length(analysisParams.targetConditions) == 2
  avgdata2=zeros(spectral_bins, num_channels, countcond2);
end

% go through all trials and get data
if verbose
  fprintf(1, 'Calculating spectra for all trials ...\n');
end
countall=0;
countcond1=0;
countcond2=0;
for cur_trial=min(trials)+1:max(trials)-1
 if verbose && (mod(cur_trial+1, 50) == 0)
    fprintf(1, '%03d ', cur_trial+1);
    if (mod(cur_trial+1, 150) == 0)
       fprintf(1, '* /%d\r', max(trials));
    end
 end

 % condition 1
 eval(condition1idxstr);
 if (isempty(condition1idx) == 0)
    % get the data for these samples
    condition1data=double(signal(condition1idx, :));
    datalength=size(condition1data, 1);
    cur_start=1;
    count=0;
    trialspectrum=zeros(spectral_bins, num_channels);
    while (cur_start+spectral_size <= datalength)
     [cur_spectrum, freq_bins]=mem(double(condition1data(cur_start:cur_start+spectral_size, :)), parms);
     trialspectrum=trialspectrum+cur_spectrum;
     countall=countall+1;
     cur_start=cur_start+spectral_stepping;
     count=count+1;
    end
    if (count > 0)
       trialspectrum=trialspectrum/count;
       countcond1=countcond1+1;
       avgdata1(:, :, countcond1)=trialspectrum;
    end
 end

 if length(analysisParams.targetConditions) == 2
   % condition 2
   eval(condition2idxstr);
   if (isempty(condition2idx) == 0)
      % get the data for these samples
      condition2data=double(signal(condition2idx, :));
      datalength=size(condition2data, 1);
      cur_start=1;
      count=0;
      trialspectrum=zeros(spectral_bins, num_channels);
      while (cur_start+spectral_size <= datalength)
       [cur_spectrum, freq_bins]=mem(double(condition2data(cur_start:cur_start+spectral_size, :)), parms);
       trialspectrum=trialspectrum+cur_spectrum;
       countall=countall+1;
       cur_start=cur_start+spectral_stepping;
       count=count+1;
      end
      if (count > 0)
         trialspectrum=trialspectrum/count;
         countcond2=countcond2+1;
         avgdata2(:, :, countcond2)=trialspectrum;
      end
   end
  end % session
end

if countcond1 < minTrialsPerCondErr
  error([funcName ':minTrialsCond1NotMet'], sprintf('Evaluation of target condition 1 in conjunction with the trial change condition resulted in fewer than %d useable trials.  Please relax your conditions.', minTrialsPerCondErr));
elseif ~errorOverride && countcond1 < minTrialsPerCondWarn
  error([funcName ':sugTrialsCond1NotMet'], sprintf('Evaluation of target condition 1 in conjunction with the trial change condition resulted in fewer than %d useable trials.  You may want to relax your conditions.', minTrialsPerCondWarn));
end
if length(analysisParams.targetConditions) == 2
  if countcond2 < minTrialsPerCondErr
    error([funcName ':minTrialsCond2NotMet'], sprintf('Evaluation of target condition 2 in conjunction with the trial change condition resulted in fewer than %d useable trials.  Please relax your conditions.', minTrialsPerCondErr));
  elseif ~errorOverride && countcond2 < minTrialsPerCondWarn
    error([funcName ':sugTrialsCond2NotMet'], sprintf('Evaluation of target condition 2 in conjunction with the trial change condition resulted in fewer than %d useable trials.  You may want to relax your conditions.', minTrialsPerCondWarn));
  end
end

if verbose
  fprintf(1, '\r');
  fprintf(1, 'Calculating r^2 statistics ...\n');
end

% calculate average spectra for each condition and each channel
res1=mean(avgdata1, 3);
res2 = [];
ressq = [];

if length(analysisParams.targetConditions) == 2
  % calculate rvalue/rsqu for each channel and each spectral bin between the
  % two conditions
  ressq = calc_rsqu(double(avgdata1), double(avgdata2), 1);

  res2=mean(avgdata2, 3);
end


freq_bins = freq_bins - binwidth/2;
