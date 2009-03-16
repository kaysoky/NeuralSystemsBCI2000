#   $Id$
#   
#   This file is part of the BCPy2000 framework, a Python framework for
#   implementing modules that run on top of the BCI2000 <http://bci2000.org/>
#   platform, for the purpose of realtime biosignal processing.
# 
#   Copyright (C) 2007-9  Jeremy Hill, Thomas Schreiner,
#                         Christian Puzicha, Jason Farquhar
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
__all__ = ['bcibar', 'addbar', 'updatebars', 'EQBars']

##############################################################################################
# feature-meters!
##############################################################################################

import os
import numpy
import pygame
import VisionEgg.Core
import ScreenCoordinates

	
class bcibar(object):
	def __init__(self, color=(0,0,1), font_size=26, pos=None, thickness=10, fliplr=False, fac=200.0/10.0, horiz=False, fmt='%+.2f', font_name=None):
		if font_name == None:
			fonts = ('lucida console', 'monaco', 'courier new', 'courier')
			font_name = (filter(None,map(pygame.font.match_font,fonts))+[None])[0]

		opposite = {'left':'right', 'right':'left', 'top':'bottom', 'bottom':'top'}
		txtanchor = 'right'
		txtpos = (pos[0]-font_size*0.6, pos[1])
		barpos = tuple(pos)
		if fliplr:
			txtpos,barpos = barpos,txtpos
			txtanchor = opposite[txtanchor]
		if horiz: baranchor = opposite[txtanchor]
		else:     baranchor = 'bottom'
		vetext = VisionEgg.Text.Text(position=txtpos,  anchor=txtanchor, on=True, text=' ??? ', font_size=font_size, color=color, font_name=font_name)
		verect = VisionEgg.Core.FixationSpot(position=barpos, anchor=baranchor, on=True, size=(1,1), color=color)
		self.__dict__.update({'fac':fac, 'horiz':horiz, 'fmt':fmt, 'thickness':thickness, 'length':0, 'vetext':vetext, 'verect':verect})  # fac is pixels/maxval
		self.set(val=0, text=' ??? ')

	def __setattr__(self, key, val):
		tp = self.vetext.parameters
		rp = self.verect.parameters
		if key == 'thickness':
			if self.horiz: rp.size = (rp.size[0], val)
			else:          rp.size = (val, rp.size[1])
		elif key == 'length':
			if self.horiz: rp.size = (val, rp.size[1])
			else:          rp.size = (rp.size[0], val)
		elif key == 'horiz':
			oldval = self.__dict__[key]
			if bool(val) != bool(oldval): rp.size = (rp.size[1], rp.size[0])
			opposite = {'left':'right', 'right':'left', 'top':'bottom', 'bottom':'top'}
			if val: rp.anchor = opposite[tp.anchor]
			else:   rp.anchor = 'bottom'
		if hasattr(self, key):
			self.__dict__[key] = val
		else:
			good = False
			if hasattr(tp, key): setattr(tp, key, val); good=True
			if hasattr(rp, key): setattr(rp, key, val); good=True
			if not good: raise AttributeError, 'Object has no attribute' + key
		
	def set(self, val, text=None):
		if val==None:  length = self.length
		else:          length = float(val) * float(self.fac)
		self.thickness = self.thickness # updates underlying visionegg parameter value
		self.length = length  # updates underlying visionegg parameter value
		if text==None: text = val
		if text != None:
			if not isinstance(text, str): text = self.fmt % text
			self.vetext.parameters.text = text
		
def addbar(bci, *pargs, **kwargs):
	defaultfontsize = 20
	if hasattr(bci, 'monofont') and not kwargs.has_key('font_name'):
		kwargs['font_name'] = bci.monofont
		defaultfontsize = 13
	if not kwargs.has_key('font_size'):
		kwargs['font_size'] = defaultfontsize
	b = bcibar(*pargs, **kwargs)
	if not hasattr(bci, 'bars'): bci.bars = []
	ind = len(bci.bars) + 1
	bci.stimulus('bartext_'+str(ind), b.vetext)
	bci.stimulus('barrect_'+str(ind), b.verect)
	bci.bars.append(b)
	return b
	
def updatebars(bci, vals):
	if not hasattr(bci, 'bars'): return
	if isinstance(vals, (float,int)): vals = [vals]
	if isinstance(vals, numpy.matrix): vals = vals.A
	if isinstance(vals, numpy.ndarray): vals = vals.ravel().tolist()
	for i in range(min(len(bci.bars), len(vals))): bci.bars[i].set(vals[i])


class EQBars(object):
	def __init__(self, bci, freqs, boundingbox=(0.9,0.6), maxval=800.0, color=(1,0,0), font_size=30, thickness=0.4):
		if isinstance(boundingbox, (tuple,list,float,int)):
			scaling = boundingbox
			boundingbox = ScreenCoordinates.default_coordinate_frame.copy()
			boundingbox.anchor='bottom'
			boundingbox.scale(scaling)
		boundingbox = boundingbox.copy()
		boundingbox.internal.rect = [0,0,1,1]
		self.freqs = numpy.array(freqs).flatten()
		self.maxval = float(maxval)
		self.bars = [[], []]
		nfreqs = len(self.freqs)
		y = numpy.arange(1,nfreqs+1) / float(nfreqs+1)
		y = map(lambda h:boundingbox.map(h,'y'), y)
		thickness *= boundingbox.map(1.0/float(nfreqs), 'height')
		leftbase = boundingbox.map(0.5, 'x') - font_size * 2.0
		rightbase = boundingbox.map(0.5, 'x') + font_size * 1.5
		lfac = (leftbase - boundingbox.left) / self.maxval
		rfac = (boundingbox.right - rightbase) / self.maxval
				
		for i in range(nfreqs):
			b = addbar(bci, pos=(leftbase,y[i]),  thickness=thickness, horiz=1, fmt=' ', font_size=font_size, color=color, fac=lfac, fliplr=True)  # left bar
			self.bars[0].append(b)
			b = addbar(bci, pos=(rightbase,y[i]), thickness=thickness, horiz=1, fmt=' ', font_size=font_size, color=color, fac=rfac, fliplr=False) # right bar
			self.bars[1].append(b)
			
	def update(self, freq_hz, amplitudes, side=0):
		available = numpy.array(freq_hz).flatten()
		amplitudes = numpy.array(amplitudes).flatten()			
		ind = map(lambda f:numpy.abs(f - available).argmin(), self.freqs)
		freq = map(lambda i:available[i], ind)
		amp = map(lambda i:amplitudes[i], ind)
		freqstr = map(lambda f:'% 3d Hz'%f, freq)
		for i in range(len(freq)):
			if side == 1: text = freqstr[i]
			else: text = ' '
			self.bars[side][i].set(val=min(self.maxval,amp[i]), text=text)

try:
	try: from BCI2000PythonApplication import BciGenericApplication
	except: from BCPy2000.GenericApplication import BciGenericApplication
except:
	pass
else:
	BciGenericApplication.addbar = addbar
	BciGenericApplication.updatebars = updatebars
