#!/usr/bin/python

import numpy as np

import parsematlab
import loaddata
import testweights
from iwafgui import Error, Info, SaveAs

def testWeights(name, values):
    flistwidget, fnames = values['flist']
    weightfile = values['weightfile'][1]
    if not weightfile:
        Error('You must first generate weights or select a file from which ' + \
            'to load the weights.')
        return
    errors = []
    label, value = values['test-args'][1]['matrixshape']
    matrixshape = parsematlab.parse(value.lower().replace('x', ' '))
    if isinstance(matrixshape, str):
        errors.append(label + '\n    ' + value.replace('\n', '\n    '))
    if np.isscalar(matrixshape):
        matrixshape = [matrixshape]
    label, value = values['test-args'][1]['repetitions']
    repetitions = parsematlab.parse(value)
    if isinstance(repetitions, str):
        errors.append(label + '\n    ' + value.replace('\n', '\n    '))
    if len(errors) > 0:
        Error('\n\n'.join(errors))
        return
    classifier = loaddata.load_weights(weightfile)
    if isinstance(classifier, str):
        Error(classifier)
        return
    removeanomalies = values['generation-args'][1]['removeanomalies'][1]
    data = []
    type = []
    samplingrate = None
    try:
        for fname in fnames:
            result = loaddata.load_data(fname, [0, classifier.shape[0]],
                None, True, removeanomalies = removeanomalies)
            if isinstance(result, str):
                Error(result)
                return
            if samplingrate == None:
                samplingrate = result[2]
            if samplingrate != result[2]:
                Error('Not all data files have the same sampling rate.')
                return
            data.append(result[0])
            type.append(result[1])
        if len(data) == 0 or len(type) == 0:
            Error('You must select some data upon which to test the weights.')
            return
        data = np.concatenate(data)
        type = np.concatenate(type)
        score, correctness = testweights.test_weights(data, type, classifier,
            matrixshape, repetitions)
        message = '\n'.join(fnames)
        message += '\n\n%s\n\nExpected accuracy for a %s matrix:\n\n' % \
            (
                weightfile,
                'x'.join(str(i) for i in matrixshape)
            )
        for i in range(len(repetitions)):
            if repetitions[i] != 1:
                message += '%i repetitions: %0.1f%%\n' % \
                    (repetitions[i], correctness[i] * 100)
            else:
                message += '1 repetition: %0.1f%%\n' % (correctness[i] * 100)
        message += '\nTarget STDEV: %f\nNontarget STDEV: %f\n' % score
        Info(message)
    except MemoryError:
        Error('Could not fit all the selected data in memory.\n' + \
            'Try loading fewer data files.')
        return
