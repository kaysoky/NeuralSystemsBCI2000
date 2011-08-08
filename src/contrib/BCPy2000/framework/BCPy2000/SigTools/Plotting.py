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
__all__ = [
	'plot', 'plotsig',
	'imagesc', 'scatcmp',
	'make_cmap', 'complement_cmap', 'reverse_cmap', 'show_cmaps',
	'subplots',
	'curve',
]

import numpy
from Basic import getfs,fft2ap,fft
from NumTools import isnumpyarray, project

def plot(*pargs,**kwargs):
	"""
	A wrapper around pylab.plot that reduces dimensionality-related fiddliness.
	
	plot(x, y)
	plot(y)
	
	where either x or y, independent of each other, can specify multiple lines.
	
	Additional options and their defaults:
		axis = 0         Along which dimension can one step while staying on
		                 the same graphical line?
		hold = False     If false, clear the axes before plotting.
		drawnow = True   If true, execute pylab.draw() after plotting.
	"""###
	hold = kwargs.pop('hold', False)
	axis = kwargs.pop('axis', 0)
	drawnow = kwargs.pop('drawnow', True)

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
	if drawnow: pylab.draw()
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
	try: from WavTools.Base import wav
	except ImportError: from BCPy2000.WavTools.Base import wav
	wav.plotsig = wavplotsig
except:
	pass

def imagesc(img, x=None, y=None, hold=False, drawnow=True, aspect='image', balance=None, colorbar=False, **kwargs):
	kwargs['interpolation'] = kwargs.get('interpolation', 'nearest')
	auto_aspect = {'image':False, 'auto':True}.get(aspect)
	if auto_aspect == None: raise ValueError('aspect should be "image" or "auto"')
	pylab = load_pylab()
	ax = pylab.gca()
	if not hold: ax.cla()
	
	img = numpy.asarray(img)
	if x == None and y == None:
		h = pylab.imshow(img, **kwargs)
		ax.set(xlim=(-0.5,img.shape[1]-0.5),ylim=(-0.5,img.shape[0]-0.5))
	else:
		import matplotlib
		if x == None: x = numpy.arange(img.shape[1], dtype=numpy.float64)
		if y == None: y = numpy.arange(img.shape[0], dtype=numpy.float64)
		x = numpy.asarray(x).flatten()
		y = numpy.asarray(y).flatten()
		xl = [x[0] - 0.5 * (x[1]-x[0]),   x[-1] + 0.5 * (x[-1]-x[-2])]
		yl = [y[0] - 0.5 * (y[1]-y[0]),   y[-1] + 0.5 * (y[-1]-y[-2])]
		#h = pylab.pcolor(x, y, img, edgecolors='None')
		h = matplotlib.image.NonUniformImage(ax, extent=sorted(xl)+sorted(yl))
		h.set(**kwargs)
		h.set_data(x, y, img)
		ax.images.append(h)
		ax.set(xlim=xl, ylim=yl)
	if auto_aspect: ax.set_aspect('auto')
	else: ax.set_aspect('equal')
	if balance != None:
		c = numpy.array(h.get_clim())
		c = balance + numpy.array([-1,+1]) * max(abs(c - balance))
		h.set_clim(c)
	cb = getattr(ax, 'colorbar', None)
	if colorbar:
		if cb == None: cbax = None
		else: cbax = cb.ax; cbax.cla()
		ax.colorbar = pylab.colorbar(cax=cbax, ax=ax)
	elif cb != None:
		pass # TODO: delete the colorbar---but how?
	if drawnow: pylab.draw()
	return h

def scatcmp(a, b, hold=False, drawnow=True, **kwargs):
	kwargs['linestyle'] = kwargs.get('linestyle', 'None')
	grid = kwargs.pop('grid', True)
	grid = {'on':True, 'off':False}.get(grid,grid)
	pylab = load_pylab()
	if not hold: pylab.cla()
	h = plot(a, b, drawnow=False, **kwargs)
	import matplotlib
	mm = matplotlib.lines.lineMarkers
	morder = sorted([m for m in mm if isinstance(m, basestring) and len(m.strip())]) + sorted([m for m in mm if not isinstance(m, basestring)])
	for i,hi in enumerate(h): hi.set_marker(morder[i % len(morder)])
	ax = pylab.gca()
	lim = ax.get_xlim() + ax.get_ylim()
	lim = (min(lim),max(lim))
	ax.set_xlim(lim)
	ax.set_ylim(lim)
	ax.grid(grid)
	pylab.plot(lim, lim, linestyle='-', color=(0.6,0.6,0.6), scalex=False, scaley=False)
	if drawnow: pylab.draw()
	return h

def load_pylab():
	try:
		import matplotlib,sys
		set_interactive = ('matplotlib.backends' not in sys.modules)
		if set_interactive: matplotlib.interactive(True)
		import pylab
	except:
		print __name__, "module failed to import pylab: plotting methods will not work"
	else:
		if set_interactive and hasattr(pylab, 'ion'): pylab.ion()
		return pylab

def make_cmap(cmap, name, n=256):
	pylab = load_pylab(); import matplotlib
	cmap = matplotlib.colors.LinearSegmentedColormap(name, cmap, n)
	try: matplotlib.cm.register_cmap(name=name, cmap=cmap)
	except: print "failed to register colormap '%s'" % name
	return cmap

def complement_cmap(cmap, name=None, n=256):
	pylab = load_pylab()
	if isinstance(cmap, basestring): cmap = pylab.cm.get_cmap(cmap)
	cmap = getattr(cmap, '_segmentdata', cmap)
	out = {}
	for k in ('red', 'green', 'blue'):
		v = numpy.asarray(cmap[k])
		v[:, 1:] = 1.0 - v[:, 1:]
		out[k] = v
	if name != None: out = make_cmap(out, name=name, n=n)
	return out

def reverse_cmap(cmap, name=None, n=256):
	pylab = load_pylab()
	if isinstance(cmap, basestring): cmap = pylab.cm.get_cmap(cmap)
	cmap = getattr(cmap, '_segmentdata', cmap)
	out = pylab.cm.revcmap(cmap)
	if name != None: out = make_cmap(out, name=name, n=n)
	return out
	
def show_cmaps(*pargs):
	pylab = load_pylab()
	cmaps = []
	for arg in pargs:
		if isinstance(arg, basestring): arg = arg.split()
		if isinstance(arg, (tuple,list)): cmaps += arg
		else: cmaps.append(arg)
	if len(cmaps) == 0: cmaps=sorted([m for m in pylab.cm.cmap_d if not m.endswith("_r")])
	pylab.clf()
	for i,cmap in enumerate(cmaps):
		pylab.subplot(len(cmaps), 1, i+1)
		if isinstance(cmap, basestring): cmap = pylab.cm.get_cmap(cmap)
		pylab.imshow([numpy.linspace(0.0,1.0,256,endpoint=True)], cmap=cmap)
		pylab.gca().set(aspect='auto', yticks=[0.0], yticklabels=[cmap.name])
	pylab.draw()

def subplots(r, c=None, fig=None, **kwargs):
	pylab = load_pylab()
	if fig == None: fig = pylab.gcf()
	if c == None:
		if isinstance(r, int):
			nPlots = r
			ar = fig.get_size_inches()
			ar = float(ar[0])/float(ar[1])
			layout = numpy.r_[nPlots/ar, nPlots*ar] ** 0.5
			i = numpy.argmin(abs(layout - numpy.round(layout)))
			r = int(round(layout[i]))
			c = int(numpy.ceil(nPlots/float(r)))
			if i == 1: r,c = c,r
			while r * (c-1) >= nPlots: c -= 1
			while (r-1) * c >= nPlots: r -= 1
		else:
			r,c = r
	i = 0
	ax = []
	for ri in range(r):
		row = []; ax.append(row)
		for ci in range(c):
			row.append(pylab.subplot(r, c, i+1, **kwargs))
			i += 1
	return numpy.array(ax)
	
def make_kelvin():
	kelvin_i = {
		'red': (
			(0.000, 0.0, 0.0, ),
			(0.350, 0.0, 0.0, ),
			(0.500, 1.0, 1.0, ),
			(0.890, 1.0, 1.0, ),
			(1.000, 0.5, 0.5, ),
		),
		'green': (
			(0.000, 0.0, 0.0, ),
			(0.125, 0.0, 0.0, ),
			(0.375, 1.0, 1.0, ),
			(0.640, 1.0, 1.0, ),
			(0.910, 0.0, 0.0, ),
			(1.000, 0.0, 0.0, ),
		),
		'blue': (
			(0.000, 0.5, 0.5, ),
			(0.110, 1.0, 1.0, ),
			(0.500, 1.0, 1.0, ),
			(0.650, 0.0, 0.0, ),
			(1.000, 0.0, 0.0, ),
		),
	}
	kelvin_i = make_cmap(kelvin_i, 'kelvin_i', 256)
	kelvin_r = complement_cmap(kelvin_i, 'kelvin_r', 256)
	kelvin   = reverse_cmap(kelvin_r, 'kelvin', 256)

make_kelvin()

def curve(x,y=None, hold=False, drawnow=True, **kwargs):
	pylab = load_pylab(); import matplotlib, matplotlib.path
	if not hold: pylab.cla()
	kwargs['facecolor'] = kwargs.get('facecolor', 'None')
	if y == None: xy = x
	else: xy = numpy.c_[x,y]
	codes = [matplotlib.path.Path.MOVETO] + [matplotlib.path.Path.CURVE4] * (len(xy)-1)
	path = matplotlib.path.Path(xy,codes)
	patch = matplotlib.patches.PathPatch(path, lw=2, **kwargs)
	pylab.gca().add_patch(patch)
	if drawnow: pylab.draw()

