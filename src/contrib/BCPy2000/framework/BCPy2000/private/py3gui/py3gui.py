#!/usr/bin/python

temp = __import__('Tkinter').Tk()
temp.title('Please Wait...')
temp.update()

import numpy as np
import pylab
pylab.ion()

import parsematlab
import loaddata
import testweights
import swlda
from iwafgui import Iwaf, FileList, Arguments, Action, Browse, Quit, Error, \
    Info, SaveAs

temp.destroy()
del temp

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
    #reload(loaddata) #TODO!!!
    flistwidget, fnames = values['flist']
    weightfile = values['weightfile'][1]
    if not weightfile:
        Error('You must first generate weights or select a file from which ' + \
            'to load the weights.')
        return
    filetype = values['generation-args'][1]['filetype'][1]
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
    for fname in fnames:
        result = loaddata.load_data(fname, [0, classifier.shape[0]],
            filetype, True, removeanomalies = removeanomalies)
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

def generateFeatureWeights(name, values):
    #reload(loaddata) #TODO!!!
    args = values['generation-args'][1]
    errors = []
    for key in args:
        if key in ('filetype', 'removeanomalies'):
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
    filetype = args['filetype'][1]
    weightwidget = values['weightfile'][0]
    removeanomalies = args['removeanomalies'][1]
    data = []
    type = []
    samplingrate = None
    for fname in fnames:
        result = loaddata.load_data(fname, response_window, filetype,
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
            Error('"Channel Set" is not a subset of the available channels.')
            return
        type.append(result[1])
    if len(data) == 0 or len(type) == 0:
        return
    data = np.concatenate(data)
    type = np.concatenate(type)
    randomindices = np.arange(data.shape[0], dtype = int)
    np.random.shuffle(randomindices)
    randomindices = randomindices[:data.shape[0] * random_sample_percent // 100]
    randomindices.sort()
    data = data[randomindices]
    type = type[randomindices]
    result = swlda.swlda(data, type, samplingrate, response_window,
        decimation_frequency, max_model_features, penter, premove)
    if isinstance(result, str):
        Error(result)
        return
    channels, weights = result
    prm = exportToPRM(channels, weights, response_window[1])
    try:
        fname = SaveAs()
        if fname:
            prmfile = open(fname, 'wb')
            prmfile.write(prm)
            prmfile.close()
            weightwidget.setContents(fname)
    except:
        Error('Could not write PRM file.')
        return

def plotWaveform(name, values):
    #reload(loaddata) #TODO!!!
    args = values['generation-args'][1]
    errors = []
    for key in ['responsewindow', 'channelset', 'filetype']:
        if key == 'filetype':
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
    channelset = args['channelset'] - 1
    fnames = values['flist'][1]
    filetype = args['filetype'][1]
    removeanomalies = args['removeanomalies'][1]
    data = []
    type = []
    samplingrate = None
    for fname in fnames:
        result = loaddata.load_data(fname, response_window, filetype,
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
            Error('"Channel Set" is not a subset of the available channels.')
            return
        type.append(result[1])
    if len(data) == 0 or len(type) == 0:
        return
    data = np.concatenate(data)
    type = np.concatenate(type)
    target = data[type.nonzero()[0]].mean(axis = 0)
    nontarget = data[(~type).nonzero()[0]].mean(axis = 0)
    ylim = [min(target.min(), nontarget.min()),
        max(target.max(), nontarget.max())]
    pylab.figure()
    ax = pylab.subplot(2, 1, 1)
    pylab.title('Target')
    pylab.plot(target)
    pylab.ylim(ylim)
    pylab.subplot(2, 1, 2, sharex = ax, sharey = ax)
    pylab.title('Non-Target')
    pylab.plot(nontarget)
    pylab.ylim(ylim)

def main(argv = []):
    Iwaf(
        title = 'Py3GUI',
        size = (600, 500),
        contents = [
            FileList('flist', 'Select Training Data'),
            Arguments(
                'generation-args',
                [
                    ('responsewindow', 'Response Window [begin end] (ms): ',
                        '0 800'),
                    ('randompercent', '% Random Sample for Training: ',
                        '100'),
                    ('decimationfrequency', 'Decimation Frequency (Hz): ',
                        '20'),
                    ('maxmodelfeatures', 'Max Model Features: ',
                        '60'),
                    ('channelset', 'Channel Set: ', '1:8'),
                    ('penter', 'Threshold to Add Features: ', '0.1'),
                    ('premove', 'Threshold to Remove Features: ', '0.15'),
                    ('filetype', 'Data File Type: ', loaddata.SUPPORTED),
                    ('removeanomalies', 'Attempt to Remove Anomalies', False),
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
