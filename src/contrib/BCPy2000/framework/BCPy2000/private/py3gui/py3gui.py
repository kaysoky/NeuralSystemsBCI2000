#!/usr/bin/python

import os
import sys
__file__ = os.path.abspath(sys.argv[0])

if __name__ == '__main__':
    __import__('iwafgui').Splash(
        os.path.join(os.path.dirname(__file__), 'logo.gif')
    )

import pylab

from iwafgui import Iwaf, MultiBrowse, Arguments, Action, Browse, Quit
from py3testweights import testWeights
from py3generatefeatureweights import generateFeatureWeights
from py3diagnosticplot import diagnosticPlot

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
            Action('Generate Feature Weights', generateFeatureWeights),
            Browse('weightfile', 'Use weights from: '),
            Action('Diagnostic Plot', diagnosticPlot),
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
