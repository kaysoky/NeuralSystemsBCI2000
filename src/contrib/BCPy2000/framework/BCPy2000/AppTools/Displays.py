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
__all__ = ['monitor', 'number_of_monitors', 'init_screen', 'split_33_66', 'fullscreen']

import Boxes

try:
	import win32api
	def monitor(id=0):
		m = win32api.EnumDisplayMonitors()
		m = map(lambda x: Boxes.box(rect=x[2]), m)
		rebase = m[0].height
		for i in range(1,len(m)): m[i].bottom,m[i].top = rebase-m[i].top,rebase-m[i].bottom
		if id == None: return m
		else: return m[id]
	def number_of_monitors():
		return len(monitor(None))
		
except:
	print __name__,"module failed to import win32api"
	import ctypes
	GetSystemMetrics = ctypes.windll.user32.GetSystemMetrics
	def monitor(id=0):
		m = Boxes.box(rect=(0,0,GetSystemMetrics(0),GetSystemMetrics(1)))
		if id != 0: print "win32api not available---cannot get information about multiple displays"
		if id == None: return [m]
		else: return m
	def number_of_monitors():
		return 0



import os
try: import VisionEgg
except: print __name__,"module failed to import VisionEgg"
import ScreenCoordinates
		
def init_screen(b, **kwargs):
	b = b.copy()
	VisionEgg.config.VISIONEGG_SCREEN_W = int(round(b.width))
	VisionEgg.config.VISIONEGG_SCREEN_H = int(round(b.height))
	pos = [b.left, monitor(0).top - b.top]
	os.environ['SDL_VIDEO_WINDOW_POS'] = ','.join(map(lambda x: str(int(round(x))), pos))

	VisionEgg.config.VISIONEGG_HIDE_MOUSE = 0
	VisionEgg.config.VISIONEGG_GUI_INIT = 0
	VisionEgg.config.VISIONEGG_FULLSCREEN = 0
	VisionEgg.config.VISIONEGG_FRAMELESS_WINDOW = 1

	for (k,v) in kwargs.items():
		cfg = VisionEgg.config
		if not hasattr(cfg, k) and hasattr(cfg, 'VISIONEGG_'+k.upper()): k = 'VISIONEGG_'+k.upper()
		elif not hasattr(cfg, k) and hasattr(cfg, k.upper()): k = k.upper()
		setattr(cfg, k, v)
	
	b.sticky = True
	b.anchor = 'bottom left'
	b.position = (0,0)
	if b.internal == None: b.internal = b.__class__(rect=(-1,-1,+1,+1))
	ScreenCoordinates.default_coordinate_frame = b
	return b

def fullscreen(scale=1.0, id=-1, anchor='center', **kwargs):
	m = monitor(id)
	m.anchor = anchor
	m.scale(scale)
	return init_screen(m, **kwargs)
	
def split_33_66(bci, **kwargs):
	m = monitor(-1)
	if number_of_monitors() == 2:
		panelwidth = monitor(0).width/3
		m.anchor = 'top right'
		m.width += panelwidth
	else:
		m.anchor = 'top right'
		m.height /= 2; m.width *=0.8
		panelwidth = m.width/3
	bci.experimenter_panel = init_screen(m, **kwargs) # this one is used by the global scr() function by default
	bci.subject_panel = bci.experimenter_panel.copy()
	bci.experimenter_panel.anchor = 'left'
	bci.experimenter_panel.width = panelwidth
	bci.subject_panel.anchor = 'right'
	bci.subject_panel.width -= panelwidth
		
