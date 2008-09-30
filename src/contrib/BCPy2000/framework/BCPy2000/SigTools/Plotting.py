#   $Id$
#   
#   This file is part of the BCPy2000 framework, a Python framework for
#   implementing modules that run on top of the BCI2000 <http://bci2000.org/>
#   platform, for the purpose of realtime biosignal processing.
# 
#   Copyright (C) 2007-8  Thomas Schreiner, Jeremy Hill
#                         Christian Puzicha, Jason Farquhar
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
__all__ = [
	'plot', 'plotsig'
]

import numpy
from Basic import getfs,fft2ap,fft
from NumTools import isnumpyarray, project

def plot(*pargs,**kwargs):
	"""
	A wrapper around pylab.plot that reduces dimensionality-related fiddliness.
	"""###
	hold = kwargs.pop('hold', False)
	axis = kwargs.pop('axis', 0)

	pargs = list(pargs) # makes a copy, at least of the list container
	allvec = True
	for i in range(len(pargs)):
		if isinstance(pargs[i], (tuple,list)):
			pargs[i] = numpy.array(pargs[i],dtype=numpy.float64)
		if isinstance(pargs[i], numpy.ndarray):
			allvec &= (max(pargs[i].shape) == numpy.prod(pargs[i].shape))
			if len(pargs[i].shape) > 1:
				pargs[i] = project(pargs[i],axis).swapaxes(0,axis)
			isinf = numpy.isinf(pargs[i])
			if numpy.any(isinf):
				pargs[i] = pargs[i].copy()
				pargs[i][isinf] = numpy.nan # crude workaround---pylab.plot can't cope with infinite values
	if allvec:
		for i in range(len(pargs)):
			if isinstance(pargs[i], numpy.matrix):  pargs[i] = pargs[i].A
			if isinstance(pargs[i], numpy.ndarray): pargs[i] = pargs[i].flatten()	
	pargs = tuple(pargs)

	pylab = load_pylab()
	if not hold: pylab.cla()
	p = pylab.plot(*pargs,**kwargs)
	pylab.draw()
	return p
	
def plotsig(x, samplingfreq_hz=None, hold=False, axis=0, welch=0, **kwargs):
	"""
	Makes two subplots, showing time-series <x> in the upper panel and its
	amplitude spectrum in the lower panel.  Set <hold> in order to re-use
	a previous figure.
	
	Any additional keyword arguments are passed through to pylab.plot for
	both subplots.
	"""###
	fs = getfs(samplingfreq_hz)
	if fs==None: fs = getfs(x,2.0)
	if hasattr(x, 'x'): x = x.x
	elif hasattr(x, 'y'): x = x.y
	
	if not isnumpyarray(x):
		axis = 0
		if isinstance(x[0], list) or isinstance(x[0], tuple): axis = 1
		x = numpy.array(x,dtype='float')

	xwin = x = project(x,axis).swapaxes(0, axis)
	nsamp = x.shape[0]
	
	class Unfinished(Exception): pass
	if welch==1: xwin = x * project(hanning(nsamp),len(x.shape)-1)
	elif welch > 0: raise Unfinished, "Welch periodogram not yet implemented"
	
	t = numpy.arange(0, nsamp) / float(fs)
	ap = fft2ap(fft(xwin,axis=0),samplingfreq_hz=fs,axis=0)
	f = ap['freq_hz']
	a = 20.0 * numpy.log10(ap['amplitude'])

	pylab = load_pylab()
	if not hold: pylab.clf()
	
	pylab.subplot(2,1,1)
	h1 = pylab.plot(t,x,**kwargs)
	ax = pylab.gca()
	ax.set_xlim(t[0], t[-1])
	ax.xaxis.grid(True)
	ax.yaxis.grid(True)
	
	pylab.subplot(2,1,2)
	a[numpy.isinf(a)] = numpy.nan # crude workaround---pylab.plot can't cope with infinite values
	h2 = pylab.plot(f,a,**kwargs)
	ax = pylab.gca()
	ax.set_xlim(f[0], f[-1])
	ax.xaxis.grid(True)
	ax.yaxis.grid(True)

	pylab.draw()

def wavplotsig(self, *pargs, **kwargs): plotsig(self, *pargs, **kwargs)
try:
	import WavTools.Base
	WavTools.Base.wav.plotsig = wavplotsig
except:
	pass

def load_pylab():
	try:
		import matplotlib,sys
		if not 'matplotlib.backends' in sys.modules: matplotlib.interactive(True)
		import pylab
		return pylab
	except:
		print __name__, "module failed to import pylab: plotting methods will not work"


