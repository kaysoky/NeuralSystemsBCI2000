#!/usr/bin/python

import numpy as np

import parsematlab
import loaddata
import swlda
import pca_based
from iwafgui import Error, Info, SaveAs

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
