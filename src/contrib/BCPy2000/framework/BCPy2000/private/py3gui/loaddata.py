#!/usr/bin/python

import numpy as np

from BCPy2000.BCI2000Tools.FileReader import bcistream, ParseParam
from BCPy2000.BCI2000Tools.DataFiles import load

__all__ = ['load_data', 'load_weights']

SUPPORTED = ['standard', 'pickle']

def removeAnomalies(data, type, cutoff_std = 6):
    target = data[type]
    nontarget = data[~type]
    bad_target_indices = \
        np.unique(
            (
                abs(target - target.mean(axis = 0)) > \
                    cutoff_std * target.std(axis = 0)
            ).nonzero()[0]
        )
    good_target_indices = np.ones(target.shape[0], dtype = bool)
    good_target_indices[bad_target_indices] = False
    bad_nontarget_indices = \
        np.unique(
            (
                abs(nontarget - nontarget.mean(axis = 0)) > \
                    cutoff_std * nontarget.std(axis = 0)
            ).nonzero()[0]
        )
    good_nontarget_indices = np.ones(nontarget.shape[0], dtype = bool)
    good_nontarget_indices[bad_nontarget_indices] = False
    data = np.concatenate(
        (target[good_target_indices],
        nontarget[good_nontarget_indices])
    )
    type = np.zeros(data.shape[0], dtype = bool)
    type[:good_target_indices.size] = True
    return data, type

def load_weights(fname):
    f = open(fname, 'rb')
    prefices = [
        'Filtering:LinearClassifier',
        'Filtering:SpatialFilter',
        'Source:Online%20Processing',
    ]
    params = {
        'Classifier': None,
        'SpatialFilter': None,
        'TransmitChList': None,
    }
    for line in f:
        if '\0' in line:
            break
        for prefix in prefices:
            if line.startswith(prefix):
                info = ParseParam(line)
                if info['name'] in params:
                    params[info['name']] = info['val']
    try:
        errormsg = ''
        for key in params:
            if params[key] == None:
                errormsg += '    Missing %s\n' % key
        if errormsg != '':
            return ('Could not find all required parameters:\n' + \
                errormsg).strip()
        params['SpatialFilter'] = np.asarray(params['SpatialFilter'],
            dtype = float)
        if len(params['SpatialFilter'].shape) != 2 or \
            params['SpatialFilter'].shape[0] != \
                params['SpatialFilter'].shape[1] or \
            (abs(params['SpatialFilter'] - \
                np.eye(params['SpatialFilter'].shape[0])) > \
                16 * np.finfo(float).eps).any():
            return 'Only identity matrices are supported for SpatialFilter.'
        params['Classifier'] = np.asarray(params['Classifier'], dtype = float)
        if len(params['Classifier'].shape) != 2 or \
            params['Classifier'].shape[1] != 4 or \
            params['Classifier'][:, 0].min() < 1 or \
            (params['Classifier'][:, 2] != 1).any():
            raise ValueError
        params['TransmitChList'] = np.asarray(params['TransmitChList'],
            dtype = int)
        if len(params['TransmitChList'].shape) != 1 or \
            params['TransmitChList'].size < np.unique(
                params['Classifier'][:, 0]).size or\
            params['TransmitChList'].size < np.max(
                params['Classifier'][:, 0]):
            raise ValueError
    except (TypeError, ValueError):
        return 'Parameter format wrong or unexpected.'
    channels = params['TransmitChList'][
        params['Classifier'][:, 0].astype(int) - 1
    ] - 1
    samples = (params['Classifier'][:, 1] - 1).astype(int)
    classifier = np.zeros((samples.max() + 1, channels.max() + 1))
    classifier[samples, channels] = params['Classifier'][:, 3]
    return classifier

def get_state_changes(state_array, to_value = None):
    flattened = state_array.ravel()
    if to_value == None:
        return (flattened[1:] != flattened[:-1]).nonzero()[0]
    else:
        candidates = (flattened[1:] != flattened[:-1]).nonzero()[0]
        mask = (flattened[candidates] == to_value).nonzero()[0]
        return candidates[mask]

def load_standard_data(fname, window, window_in_samples):
    dat = bcistream(fname)
    signal, states = dat.decode('all')
    samplingrate = dat.samplingrate()
    sampleblocksize = dat.params['SampleBlockSize']
    stimulustime = states['StimulusTime'].ravel()
    sourcetime = states['SourceTime'].ravel()
    sourcetimesize = dat.statedefs['SourceTime']['length']
    if window_in_samples:
        window = np.arange(int(window[1]))
    else:
        window = np.arange(int(np.round(window[1] * samplingrate / 1000)))
    signal = np.asarray(signal).transpose()
    stimulusbegin = get_state_changes(states['StimulusBegin'], to_value = 1)
    data = np.zeros((stimulusbegin.size, window.size, signal.shape[1]))
    if (sourcetime >= stimulustime).all():
        sourcetimeoffset = -sampleblocksize
    else:
        sourcetimeoffset = 0
    for i in range(stimulusbegin.size):
        index = stimulusbegin[i] - int(samplingrate / 32) #sampleblocksize
        #print sampleblocksize
        #print index
        #timediff = stimulustime[index] - sourcetime[index + sourcetimeoffset]
        #if timediff > 1 << (sourcetimesize // 2):
        #    timediff -= 1 << sourcetimesize
        #print timediff
        #timediff = 0
        #index += int(
        #    np.round(
        #        timediff * samplingrate / 1000.
        #    )
        #)
        data[i] = signal[window + index, :]
    type = states['StimulusType'].ravel()[stimulusbegin] > 0
    return data, type, samplingrate

def load_pickle_data(fname, window, window_in_samples):
    pickle = load(fname)
    samplingrate = int(pickle['fs'])
    if window_in_samples:
        window = int(window[1])
    else:
        window = int(np.round(window[1] * samplingrate / 1000))
    type = pickle['y'] > 0
    data = np.swapaxes(pickle['x'], 1, 2)[:, :window, :]
    if data.shape[1] != window:
        return 'Not enough data to fill window. Window is too big.'
    return data, type, samplingrate

def load_data(fname, window, ftype = 'standard', window_in_samples = False,
    removeanomalies = False):
    reload(__import__('testweights')) #TODO!!!
    try:
        if ftype == 'standard':
            data, type, samplingrate = load_standard_data(fname, window,
                window_in_samples)
        elif ftype == 'pickle':
            data, type, samplingrate = load_pickle_data(fname, window,
                window_in_samples)
        else:
            return '%s file type not supported.' % str(ftype)
        if removeanomalies:
            data, type = removeAnomalies(data, type)
        return data, type, samplingrate
    except KeyError:
        return 'Data could not be loaded. Wrong file type selected?'

def main(argv = []):
    load_data(
        '/Documents and Settings/bci/Desktop/side-by-side/20100714_JV_002/20100714_JV_S002R01.pk',
        [0, 600],
        'pickle'
    )
    load_data(
        '/Documents and Settings/bci/Desktop/side-by-side/20100714_JV_001/20100714_JV_S001R02.dat',
        [0, 800],
        'standard'
    )

if __name__ == '__main__':
    import sys
    main(sys.argv[1:])
