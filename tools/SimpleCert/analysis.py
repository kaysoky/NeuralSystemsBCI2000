import os, sys, pylab

import BCPy2000.Paths
from BCI2000Tools.TimingAnalysis import StimulusTiming

datadir = sys.argv[1]
StimulusTiming( datadir, channels=[0,1] )

outputfile = os.path.join( datadir, 'timing.pdf' )
fig = pylab.gcf()
fig.savefig( outputfile )

# note that the earliest stimulus is at the bottom
# of the plot, not the top as in matlab