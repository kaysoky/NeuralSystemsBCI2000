#!/usr/bin/python

import numpy as np
from scipy import stats

from convolution import convolve, max_gauss_pdf, max_gauss_cdf

__all__ = ['test_weights']

def accuracy(nt_mean, nt_var, nt_count, t_mean, t_var, repetitions):
    nt_mean *= repetitions
    nt_var *= repetitions
    nt_std = np.sqrt(nt_var)
    t_mean *= repetitions
    t_var *= repetitions
    t_std = np.sqrt(t_var)
    return convolve(
        lambda t: max_gauss_cdf(nt_mean - t_mean, nt_std, nt_count, t),
        lambda t: max_gauss_pdf(0, t_std, 1, t),
        0
    )

def test_weights(responses, type, classifier, matrixshape, repetitions):
    responses = np.asarray(responses, dtype = float) \
        [:, :classifier.shape[0], :classifier.shape[1]]
        # If the weights do not include the last channel or two (or more),
        # then the dense classifier matrix created will not be the right
        # dimensions. Since this only occurs for the last channels, this
        # can be corrected by throwing out the channels that are not in
        # the classification matrix, as done by [..., :classifier.shape[1]]
    type = np.asarray(type, dtype = bool)
    if responses.shape[1] != classifier.shape[0]:
        return 'Response window not long enough.'
    target = type.nonzero()[0]
    nontarget = (~type).nonzero()[0]
    target_scores = (responses[target] * classifier). \
        sum(axis = 1).sum(axis = 1)
    target_mean = target_scores.mean()
    target_var = target_scores.var(ddof = 1)
    nontarget_scores = (responses[nontarget] * classifier). \
        sum(axis = 1).sum(axis = 1)
    nontarget_mean = nontarget_scores.mean()
    nontarget_var = nontarget_scores.var(ddof = 1)
    if target_mean <= nontarget_mean:
        return 'These weights are so bad that they actually ' + \
            'classify *against* the target.'
    if np.isscalar(matrixshape):
        matrixshape = [matrixshape]
    if np.isscalar(repetitions):
        repetitions = [repetitions]
    correctness = []
    for reps in repetitions:
        total_accuracy = 1
        for dimension in matrixshape:
            total_accuracy *= \
                accuracy(
                    nontarget_mean,
                    nontarget_var,
                    dimension - 1,
                    target_mean,
                    target_var,
                    reps
                )
        correctness.append(total_accuracy)
    c_mean = target_mean - nontarget_mean
    target_std = np.sqrt(target_var) / c_mean
    nontarget_std = np.sqrt(nontarget_var) / c_mean
    return (target_std, nontarget_std), correctness
