#!/usr/bin/python

import numpy as np
from scipy import stats
import pylab
pylab.ion()

import parsematlab
import loaddata
from iwafgui import Error, Info, SaveAs

def diagnosticPlot(name, values):
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
    weightfile = values['weightfile'][1]
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
        if weightfile:
            weights = loaddata.load_weights(weightfile)
            classifier = np.zeros(data.shape[1:])
            classifier[:weights.shape[0], :weights.shape[1]] = weights
            classifier_max = max(abs(classifier.max()), abs(classifier.min()))
        else:
            classifier = None
        if isinstance(classifier, str):
            Error(classifier)
            return
        num_plots = 3 if classifier == None else 4
        signed_r = np.zeros(data.shape[1:])
        for row in range(signed_r.shape[0]):
            for col in range(signed_r.shape[1]):
                signed_r[row, col] = stats.linregress(
                    data[:, row, col], type
                )[2]
        signed_r_max = max(abs(signed_r.max()), abs(signed_r.min()))
        x = np.arange(data.shape[1]) * 1000 / samplingrate
        target = data[type.nonzero()[0]].mean(axis = 0)
        nontarget = data[(~type).nonzero()[0]].mean(axis = 0)
        vmin, vmax = ylim = [min(target.min(), nontarget.min()),
            max(target.max(), nontarget.max())]
        fig = pylab.figure()
        fig.subplots_adjust(bottom = 0.06, top = 0.93, hspace = 0.45)
        master_ax = ax = pylab.subplot(num_plots, 1, 1)
        ax.callbacks.connect(
            'ylim_changed',
            lambda ax: ax.set_ylim(-0.5, data.shape[2] - 0.5) if \
                ax.get_ylim() != (-0.5, data.shape[2] - 0.5) else None
        )
        pylab.title('Target', fontsize = 'medium')
        pylab.imshow(target.transpose(), interpolation = 'nearest',
            cmap = 'PRGn', aspect = 'auto', vmin = vmin, vmax = vmax,
            origin = 'lower', extent = (
                0,
                data.shape[1] * 1000 / samplingrate,
                -0.5,
                data.shape[2] - 0.5
            )
        )
        pylab.xticks(fontsize = 'small')
        pylab.yticks(range(data.shape[2]),
            [str(i) for i in range(1, data.shape[2] + 1)],
            fontsize = 'small')
        pylab.axes(pylab.colorbar().ax)
        pylab.yticks(fontsize = 'small')
        ax = pylab.subplot(num_plots, 1, 2, sharex = master_ax)
        ax.callbacks.connect(
            'ylim_changed',
            lambda ax: ax.set_ylim(-0.5, data.shape[2] - 0.5) if \
                ax.get_ylim() != (-0.5, data.shape[2] - 0.5) else None
        )
        pylab.title('Non-Target', fontsize = 'medium')
        pylab.imshow(nontarget.transpose(), interpolation = 'nearest',
            cmap = 'PRGn', aspect = 'auto', vmin = vmin, vmax = vmax,
            origin = 'lower', extent = (
                0,
                data.shape[1] * 1000 / samplingrate,
                -0.5,
                data.shape[2] - 0.5
            )
        )
        pylab.xticks(fontsize = 'small')
        pylab.yticks(range(data.shape[2]),
            [str(i) for i in range(1, data.shape[2] + 1)],
            fontsize = 'small')
        pylab.axes(pylab.colorbar().ax)
        pylab.yticks(fontsize = 'small')
        ax = pylab.subplot(num_plots, 1, 3, sharex = master_ax)
        ax.callbacks.connect(
            'ylim_changed',
            lambda ax: ax.set_ylim(-0.5, data.shape[2] - 0.5) if \
                ax.get_ylim() != (-0.5, data.shape[2] - 0.5) else None
        )
        pylab.title('Correlation Coefficient', fontsize = 'medium')
        pylab.imshow(signed_r.transpose(), interpolation = 'nearest',
            cmap = 'PRGn', aspect = 'auto', vmin = -signed_r_max,
            vmax = signed_r_max, origin = 'lower', extent = (
                0,
                data.shape[1] * 1000 / samplingrate,
                -0.5,
                data.shape[2] - 0.5
            )
        )
        pylab.xticks(fontsize = 'small')
        pylab.yticks(range(data.shape[2]),
            [str(i) for i in range(1, data.shape[2] + 1)],
            fontsize = 'small')
        pylab.axes(pylab.colorbar().ax)
        pylab.yticks(fontsize = 'small')

        if classifier == None:
            return

        ax = pylab.subplot(num_plots, 1, 4, sharex = master_ax)
        ax.callbacks.connect(
            'ylim_changed',
            lambda ax: ax.set_ylim(-0.5, data.shape[2] - 0.5) if \
                ax.get_ylim() != (-0.5, data.shape[2] - 0.5) else None
        )
        pylab.title('Classifier Weights', fontsize = 'medium')
        pylab.imshow(classifier.transpose(), interpolation = 'nearest',
            cmap = 'PRGn', aspect = 'auto', vmin = -classifier_max,
            vmax = classifier_max, origin = 'lower', extent = (
                0,
                data.shape[1] * 1000 / samplingrate,
                -0.5,
                data.shape[2] - 0.5
            )
        )
        pylab.xticks(fontsize = 'small')
        pylab.yticks(range(data.shape[2]),
            [str(i) for i in range(1, data.shape[2] + 1)],
            fontsize = 'small')
        pylab.axes(pylab.colorbar().ax)
        pylab.yticks(fontsize = 'small')
    except MemoryError:
        Error('Could not fit all the selected data in memory.\n' + \
            'Try loading fewer data files.')
        return
