# -*- coding: utf-8 -*-
# 
#   $Id$
#   
#   This file is part of the BCPy2000 framework, a Python framework for
#   implementing modules that run on top of the BCI2000 <http://bci2000.org/>
#   platform, for the purpose of realtime biosignal processing.
# 
#   Copyright (C) 2007-11  Jeremy Hill, Thomas Schreiner,
#                          Christian Puzicha, Jason Farquhar
#   
#   bcpy2000@bci2000.org
#   
#   The BCPy2000 framework is free software: you can redistribute it
#   and/or modify it under the terms of the GNU General Public License
#   as published by the Free Software Foundation, either version 3 of
#   the License, or (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
__all__ = ['TimingWindow']

import os, sys, time, numpy, pylab

from FileReader import bcistream

try: from SigTools import sstruct, plot, samples2msec, unwrapdiff
except ImportError: from BCPy2000.SigTools import sstruct, plot, samples2msec, unwrapdiff


def TimingWindow(filename='.', ind=-1, save=None):
	"""
Recreate BCI2000's timing window offline, from a saved .dat file specified by <filename>.
It is also possible to supply a directory name as <filename>, and an index <ind> (default
value -1 meaning "the last run") to choose a file automatically from that directory.

Based on BCI2000's   src/shared/modules/signalsource/DataIOFilter.cpp where the timing window
content is computed in DataIOFilter::Process(), this is what appears to happen:
    
         Begin SampleBlock #t:
            Enter SignalSource module's first Process() method (DataIOFilter::Process)
            Save previous SampleBlock to file
            Wait to acquire new SampleBlock from hardware
 +--------- Measure SourceTime in SignalSource module
 |   |   |  Make a local copy of all states (NB: all except SourceTime were set during #t-1) ---+
B|  R|  S|  Pipe the signal through the rest of BCI2000                                         |
 |   |   +- Measure StimulusTime in Application module, on leaving last Process() method        |
 |   |                                                                                          |
 |   |                                                                                          |
 |   |   Begin SampleBlock #t+1:                                                                |
 |   +----- Enter SignalSource module's first Process() method (DataIOFilter::Process)          |
 |          Save data from #t, SourceTime state from #t, and other states from #t-1, to file <--+
 |          Wait to acquire new SampleBlock from hardware
 +--------- Measure SourceTime in SignalSource module
            Make a local copy of all states (NB: all except SourceTime were set during #t)
            Leave DataIOFilter::Process() and pipe the signal through the rest of BCI2000
            Measure StimulusTime in Application module, on leaving last Process() method

B stands for Block duration.
R stands for Roundtrip time (visible in VisualizeTiming, not reconstructable from the .dat file)
S is the filter cascade time (marked "Stimulus" in the VisualizeTiming window).

Note that, on any given SampleBlock as saved in the file, SourceTime will be *greater* than
any other timestamp states (including StimulusTime), because it is the only state that is
updated in time to be saved with the data packet it belongs to. All the others lag by one
packet.  This is corrected for at the point commented with ??? in the Python code. 
"""
	
	if hasattr(filename, 'filename'): filename = filename.filename
	
	b = bcistream(filename=filename, ind=ind)
		
	out = sstruct()
	out.filename = b.filename
	#print "decoding..."
	sig,states = b.decode('all')
	#print "done"
	b.close()

	dT,T,rT = {},{},{}
	statenames = ['SourceTime', 'StimulusTime'] + ['PythonTiming%02d' % (x+1) for x in range(2)]
	statenames = [s for s in statenames if s in states]
	for key in statenames:
		dT[key],T[key] = unwrapdiff(states[key].flatten(), base=65536, dtype=numpy.float64)

	sel, = numpy.where(dT['SourceTime'])
	for key in statenames:
		dT[key] = dT[key][sel[1:]]
		if key == 'SourceTime': tsel = sel[:-1]  # ??? why the shift
		else:                   tsel = sel[1:]   # ??? relative to here?
		T[key] = T[key][tsel+1]

	t0 = T['SourceTime'][0]
	for key in statenames: T[key] -= t0

	t = T['SourceTime'] / 1000

	expected = b.samples2msec(b.params['SampleBlockSize'])
	datestamp = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(b.datestamp))
	paramstr = ', '.join(['%s=%s' % (x,b.params[x]) for x in ['SampleBlockSize', 'SamplingRate', 'VisualizeTiming', 'VisualizeSource']])
	chainstr = '-'.join([x for x,y in b.params['SignalSourceFilterChain']+b.params['SignalProcessingFilterChain']+b.params['ApplicationFilterChain']])
	titlestr = '\n'.join([b.filename, datestamp, paramstr, chainstr])

	plot(t[[0,-1]], [expected]*2, drawnow=False)
	plot(t, dT['SourceTime'], hold=True, drawnow=False)

	for key in statenames:
		if key == 'SourceTime': continue
		rT[key] = T[key] - T['SourceTime']
		plot(t, rT[key], hold=True, drawnow=False)
	
	import pylab
	pylab.title(titlestr)
	pylab.grid(True)
	pylab.xlabel('seconds')
	pylab.ylabel('milliseconds')
	ymin,ymax = pylab.ylim(); pylab.ylim(ymax=max(ymax,expected*2))
	pylab.xlim(xmax=t[-1])
	pylab.draw()
	out.params = sstruct(b.params)
	out.summarystr = titlestr
	out.t = t
	out.SourceTime = T['SourceTime']
	out.StimulusTime = T['StimulusTime']
	out.BlockDuration = dT['SourceTime']
	out.BlockDuration2 = dT['StimulusTime']
	out.ProcessingTime = out.StimulusTime - out.SourceTime
	out.ExpectedBlockDuration = expected
	out.rT = rT
	out.dT = dT
	out.T = T
	
	if save:
		pylab.gcf().savefig(save, orientation='landscape')
	
	return out
	
if __name__ == '__main__':
	out = timing(sys.argv[1])
