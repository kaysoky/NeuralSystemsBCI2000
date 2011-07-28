#!/usr/bin/python

import numpy as np

from stepwise import stepwisefit

def swlda(responses, type, sampling_rate, response_window, decimation_frequency,
    max_model_features = 60, penter = 0.1, premove = 0.15):
    """
    Stepwise Linear Discriminant Analysis
    ``responses'' must be a (trials x samples x channels) array containing
    responses to a stimulus.
    ``type'' must be a one-dimensional array of bools of length trials.
    ``sampling_rate'' is the sampling rate of the data.
    ``response_window'' is of the form [begin, end] in milliseconds.
    ``decimation_frequency'' is the frequency at which to resample.
    ``max_model_features'' is the maximum allowed number of features to be
        chosen by stepwisefit.
    ``penter'' and ``premove'' are the thresholds for adding and removing
        features from the model.
    """

    # Housekeeping
    responses = np.asarray(responses, dtype = float)
    type = np.asarray(type, dtype = bool)
    response_window = np.asarray(response_window)
    if np.size(response_window) == 1:
        # Make response_window into an array of length 2.
        response_window = np.asarray([0, np.ravel(response_window)[0]])
    assert np.shape(response_window) == (2,)
    # End housekeeping

    dec_factor = int(np.round(float(sampling_rate) / decimation_frequency))
    response_window = np.asarray(np.round(
        response_window * sampling_rate / 1000.), dtype = int)
    trials, samples, channels = responses.shape

    # The following pieces of information are now known:
    #    response_window ([begin, end] in samples)
    #    sampling_rate (in Hz)
    #    decimation_frequency (in Hz)
    #    max_model_features (total number of features allowed in final model)
    #    random_sampling (% of responses to be randomly selected for creating
    #        model)
    #    dec_factor (number of samples that should be decimated into one)
    #    trials, samples, channels

    indices = np.arange(response_window[0], response_window[1] - dec_factor + 1,
        dec_factor, dtype = int)
    downsampled = np.zeros((trials, indices.size, channels))
    for i in xrange(indices.size):
        index = indices[i]
        downsampled[:, i, :] = \
            responses[:, index:index + dec_factor, :].mean(axis = 1)

    # ``downsampled'' is now (trials x indices.size x channels).

    target = type.nonzero()[0]
    nontarget = (~type).nonzero()[0]
    target_then_nontarget = np.concatenate((target, nontarget))
    unraveled_sorted = np.swapaxes(downsampled, 1, 2).reshape(
        (trials, indices.size * channels)
    )[target_then_nontarget]
    labels = type[target_then_nontarget] * 2 - 1

    # ``unraveled_sorted'' is now (trials x (indices.size * channels)) and is
    # sorted into target and non-target stimuli.
    # ``labels'' contains 1s and -1s in the order of ``unraveled_sorted''.

    b, se, pval, inmodel, stats, nextstep, history = stepwisefit(
        unraveled_sorted, labels, maxiter = max_model_features,
        penter = penter, premove = premove
    )
    if not inmodel.any():
        return 'Could not find an appropriate model.'

    b = b * 10 / abs(b).max()
    b = b.reshape((channels, -1))
    inmodel = inmodel.reshape((channels, -1)).nonzero()
    whichchannels = np.unique(inmodel[0])
    inv_channel_map = np.zeros(whichchannels.max() + 1)
    inv_channel_map[whichchannels] = np.arange(1, whichchannels.size + 1)
    # ``inv_channel_map'' contains the 1-based index of each channel at the
    # index described by that channel (and zeros everyhere else).

    weights = np.zeros((inmodel[0].size, 4))
    # ``weights'' will contain three columns: channel number, sample number,
    # and the weight as assigned by stepwisefit (after being adjusted).
    weights[:, 0] = inv_channel_map[inmodel[0]] # already 1-based
    weights[:, 1] = inmodel[1]
    weights[:, 2] = 1 # channel out (for P300, this is always 1)
    weights[:, 3] = b[inmodel]

    restored_weights = np.tile(weights, (1, dec_factor)).reshape((-1, 4))
    for i in xrange(0, restored_weights.shape[0], dec_factor):
        start_val = restored_weights[i, 1] * dec_factor + 1 # 1-based
        restored_weights[i:i + dec_factor, 1] = \
            np.arange(start_val, start_val + dec_factor)
    restored_weights = restored_weights[
        restored_weights[:, 1] <= response_window[1]
    ] # remove anything past where we actually recorded data

    # make whichchannels 1-based
    return whichchannels + 1, restored_weights

def main(argv = []):
    pass

if __name__ == '__main__':
    import sys
    main(sys.argv[1:])
