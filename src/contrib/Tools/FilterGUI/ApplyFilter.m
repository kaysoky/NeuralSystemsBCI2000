function ApplyFilter(infile, outfile, settings)
  if(~exist('load_bcidat')||~exist('save_bcidat'))
    msgbox(['Could not find load_bcidat or save_bcidat mex file.\n' ...
            'Please add the BCI2000 tools/mex directory to your Matlab path.']);
    return;
  end
  
  [signal, states, parameters] = load_bcidat(infile);
  fs = parameters.SamplingRate.NumericValue;
  signal = single(signal);
  % hp filtering
  if(settings.hp~=0)
    [b,a] = butter(1, 2*settings.hp/fs, 'high');
    signal = filter(b, a, signal);
  end
  % lp filtering
  if(settings.lp~=0)
    [b,a] = butter(2, 2*settings.lp/fs, 'low');
    signal = filter(b, a, signal);
  end
  % notch filtering
  if(settings.notch~=0)
    [b,a] = cheby1(2, 0.1, 2*settings.notch/fs*[0.9 1.1], 'stop');
    signal = filter(b, a, signal);
  end
  % saving the result
  save_bcidat(outfile, signal, states, parameters);
end
