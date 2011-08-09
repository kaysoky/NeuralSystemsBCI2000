#!/usr/bin/python

import os
import sys
__file__ = os.path.abspath(sys.argv[0])

if __name__ == '__main__':
    __import__('iwafgui').Splash(
        os.path.join(os.path.dirname(__file__), 'logo.gif')
    )

import numpy as np
from scipy import stats
import pylab
pylab.ion()

import parsematlab
import loaddata
import testweights
import swlda
import pca_based
from iwafgui import Iwaf, MultiBrowse, Arguments, Action, Browse, Quit, \
    Error, Info, SaveAs

def exportToPRM(channels, weights, epoch_length):
    return ('Filtering:LinearClassifier matrix Classifier= %(lenweights)i {' + \
        ' input%%20channel input%%20element%%20(bin) output%%20channel 4 } ' + \
        '%(weights)s // Linear classification matrix in sparse representati' + \
        'on\r\n' + \
        'Filtering:P3TemporalFilter int EpochLength= %(epochlen)ims // Leng' + \
        'th of data epoch from stimulus onset\r\n' + \
        'Filtering:SpatialFilter matrix SpatialFilter= %(lenchannels)i %(le' + \
        'nchannels)i %(eye)s // columns represent input channels, rows repr' + \
        'esent output channels\r\n' + \
        'Source:Online%%20Processing list TransmitChList= %(lenchannels)i %' + \
        '(channels)s // list of transmitted channels\r\n') % {
            'lenweights': weights.shape[0],
            'weights': ' '.join(
                [
                    '%i %i %i %f' % (
                        int(row[0]), int(row[1]), int(row[2]), row[3]
                    )
                    for row in weights
                ]
            ),
            'epochlen': epoch_length,
            'lenchannels': channels.size,
            'eye': ' '.join(['%f' % i for i in np.eye(channels.size).ravel()]),
            'channels': ' '.join(['%i' % i for i in channels]),
        }

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

def generateFeatureWeights(name, values):
    args = values['generation-args'][1]
    errors = []
    for key in args:
        if key in ('removeanomalies', 'classificationmethod'):
            continue
        label, value = args[key]
        value = parsematlab.parse(value)
        if isinstance(value, str):
            errors.append(label + '\n    ' + value.replace('\n', '\n    '))
        args[key] = value
    if len(errors) > 0:
        Error('\n\n'.join(errors))
        return
    response_window = args['responsewindow']
    decimation_frequency = args['decimationfrequency']
    max_model_features = args['maxmodelfeatures']
    penter = args['penter']
    premove = args['premove']
    random_sample_percent = args['randompercent']
    channelset = args['channelset'] - 1
    fnames = values['flist'][1]
    weightwidget = values['weightfile'][0]
    removeanomalies = args['removeanomalies'][1]
    classificationmethod = args['classificationmethod'][1]
    data = []
    type = []
    samplingrate = None
    try:
        for fname in fnames:
            result = loaddata.load_data(fname, response_window, None,
                removeanomalies = removeanomalies)
            if isinstance(result, str):
                Error(result)
                return
            if samplingrate == None:
                samplingrate = result[2]
            if samplingrate != result[2]:
                Error('Not all data files have the same sampling rate.')
                return
            try:
                data.append(result[0][:, :, channelset])
            except IndexError:
                Error('"Channel Set" is not a subset of the available ' + \
                    'channels.')
                return
            type.append(result[1])
        if len(data) == 0 or len(type) == 0:
            Error('You must select some data from which to generate ' + \
                'the weights.')
            return
        data = np.concatenate(data)
        type = np.concatenate(type)
        randomindices = np.arange(data.shape[0], dtype = int)
        np.random.shuffle(randomindices)
        randomindices = randomindices[:data.shape[0] * random_sample_percent // 100]
        randomindices.sort()
        data = data[randomindices]
        type = type[randomindices]
        if classificationmethod == 'SWLDA':
            result = swlda.swlda(data, type, samplingrate, response_window,
                decimation_frequency, max_model_features, penter, premove)
        elif classificationmethod == 'PCA-based':
            result = reload(pca_based).pca_based(data, type, samplingrate,
                response_window, decimation_frequency, max_model_features,
                penter, premove)
        if isinstance(result, str):
            Error(result)
            return
        channels, weights = result
        prm = exportToPRM(channels, weights, response_window[1])
        try:
            fname = SaveAs(filetypes = [('Parameter Files', '.prm')],
                defaultextension = 'prm')
            if fname:
                prmfile = open(fname, 'wb')
                prmfile.write(prm)
                prmfile.close()
                weightwidget.setContents(fname)
        except:
            Error('Could not write PRM file.')
            return
    except MemoryError:
        Error('Could not fit all the selected data in memory.\n' + \
            'Try loading fewer data files.')
        return

def plotWaveform(name, values):
    args = values['generation-args'][1]
    errors = []
    for key in ['responsewindow', 'channelset']:
        label, value = args[key]
        value = parsematlab.parse(value)
        if isinstance(value, str):
            errors.append(label + '\n    ' + value.replace('\n', '\n    '))
        args[key] = value
    if len(errors) > 0:
        Error('\n\n'.join(errors))
        return
    response_window = args['responsewindow']
    channelset = args['channelset'] - 1
    fnames = values['flist'][1]
    removeanomalies = args['removeanomalies'][1]
    data = []
    type = []
    samplingrate = None
    try:
        for fname in fnames:
            result = loaddata.load_data(fname, response_window, None,
                removeanomalies = removeanomalies)
            if isinstance(result, str):
                Error(result)
                return
            if samplingrate == None:
                samplingrate = result[2]
            if samplingrate != result[2]:
                Error('Not all data files have the same sampling rate.')
                return
            try:
                data.append(result[0][:, :, channelset])
            except IndexError:
                Error('"Channel Set" is not a subset of the available ' + \
                    'channels.')
                return
            type.append(result[1])
        if len(data) == 0 or len(type) == 0:
            Error('You must select some data to plot.')
            return
        data = np.concatenate(data)
        type = np.concatenate(type)
        r_squared = np.zeros(data.shape[1:])
        for row in range(r_squared.shape[0]):
            for col in range(r_squared.shape[1]):
                r_squared[row, col] = stats.linregress(
                    data[:, row, col], type
                )[2] ** 2
        r_squared_max = r_squared.max()
        x = np.arange(data.shape[1]) * 1000 / samplingrate
        target = data[type.nonzero()[0]].mean(axis = 0)
        nontarget = data[(~type).nonzero()[0]].mean(axis = 0)
        vmin, vmax = ylim = [min(target.min(), nontarget.min()),
            max(target.max(), nontarget.max())]
        fig = pylab.figure()
        fig.subplots_adjust(bottom = 0.06, top = 0.93, hspace = 0.45)
        master_ax = ax = pylab.subplot(3, 1, 1)
        ax.callbacks.connect(
            'ylim_changed',
            lambda ax: ax.set_ylim(-0.5, data.shape[2] - 0.5) if \
                ax.get_ylim() != (-0.5, data.shape[2] - 0.5) else None
        )
        pylab.title('Target')
        pylab.imshow(target.transpose(), interpolation = 'nearest',
            cmap = 'RdGy_r', aspect = 'auto', vmin = vmin, vmax = vmax,
            origin = 'lower', extent = (
                0,
                data.shape[1] * 1000 / samplingrate,
                -0.5,
                data.shape[2] - 0.5
            )
        )
        pylab.yticks(range(data.shape[2]),
            [str(i) for i in range(1, data.shape[2] + 1)])
        pylab.colorbar()
        ax = pylab.subplot(3, 1, 2, sharex = master_ax)
        ax.callbacks.connect(
            'ylim_changed',
            lambda ax: ax.set_ylim(-0.5, data.shape[2] - 0.5) if \
                ax.get_ylim() != (-0.5, data.shape[2] - 0.5) else None
        )
        pylab.title('Non-Target')
        pylab.imshow(nontarget.transpose(), interpolation = 'nearest',
            cmap = 'RdGy_r', aspect = 'auto', vmin = vmin, vmax = vmax,
            origin = 'lower', extent = (
                0,
                data.shape[1] * 1000 / samplingrate,
                -0.5,
                data.shape[2] - 0.5
            )
        )
        pylab.yticks(range(data.shape[2]),
            [str(i) for i in range(1, data.shape[2] + 1)])
        pylab.colorbar()
        ax = pylab.subplot(3, 1, 3, sharex = master_ax)
        ax.callbacks.connect(
            'ylim_changed',
            lambda ax: ax.set_ylim(-0.5, data.shape[2] - 0.5) if \
                ax.get_ylim() != (-0.5, data.shape[2] - 0.5) else None
        )
        pylab.title('$R^2$')
        pylab.imshow(r_squared.transpose(), interpolation = 'nearest',
            cmap = 'RdGy_r', aspect = 'auto', vmin = 0, vmax = r_squared_max,
            origin = 'lower', extent = (
                0,
                data.shape[1] * 1000 / samplingrate,
                -0.5,
                data.shape[2] - 0.5
            )
        )
        pylab.yticks(range(data.shape[2]),
            [str(i) for i in range(1, data.shape[2] + 1)])
        pylab.colorbar()
    except MemoryError:
        Error('Could not fit all the selected data in memory.\n' + \
            'Try loading fewer data files.')
        return

def main(argv = []):
    Iwaf(
        title = 'Py3GUI',
        size = (550, 500),
        contents = [
            MultiBrowse('flist', 'Select Training Data',
                [('Standard Data Files', '.dat'), ('Pickle Files', '.pk')]),
            Arguments(
                'generation-args',
                [
                    ('responsewindow', 'Response Window [begin end] (ms): ',
                        '0 800'),
                    ('channelset', 'Channel Set: ', '1:8'),
                    ('randompercent', '% Random Sample for Training: ',
                        '100'),
                    ('decimationfrequency', 'Decimation Frequency (Hz): ',
                        '20'),
                    ('classificationmethod', 'Classification Method: ',
                        ['SWLDA']),
                        #['SWLDA', 'PCA-based']),
                    ('maxmodelfeatures', 'Max Model Features: ',
                        '60'),
                    ('penter', 'Threshold to Add Features: ', '0.1'),
                    ('premove', 'Threshold to Remove Features: ', '0.15'),
                    ('removeanomalies', 'Attempt to Remove Anomalies: ', False),
                ]
            ),
            Action('Plot Waveform', plotWaveform),
            Action('Generate Feature Weights', generateFeatureWeights),
            Browse('weightfile', 'Use weights from: '),
            Arguments(
                'test-args',
                [
                    ('matrixshape', 'Matrix Shape: ', '6x6'),
                    ('repetitions', 'Repetitions: ', '1:15'),
                ]
            ),
            Action('Test Weights', testWeights),
            Quit(lambda: pylab.close('all') or True),
        ]
    ).mainloop()

if __name__ == '__main__':
    import sys
    main(sys.argv[1:])
