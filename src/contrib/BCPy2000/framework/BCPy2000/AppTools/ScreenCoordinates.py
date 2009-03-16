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
import Boxes

default_coordinate_frame = None

def get_default_coordinate_frame():
	if not screensizeset():
		import VisionEgg
		setscreensize(VisionEgg.config.VISIONEGG_SCREEN_W, VisionEgg.config.VISIONEGG_SCREEN_H)
	return default_coordinate_frame
	
def screensize():
	return get_default_coordinate_frame().size
	
def setscreensize(*pargs):
	if len(pargs) == 1: scrw,scrh=pargs[0]
	elif len(pargs) == 2: scrw,scrh=pargs
	else: raise TypeError, 'expected 1 or 2 input arguments'
	global default_coordinate_frame
	default_coordinate_frame = Boxes.box(rect=(0,0,scrw,scrh))
	
def screensizeset():
	return default_coordinate_frame != None
	
def scr(*pargs): # return, in pixels, a tuple corresponding to the point on the screen specified in relative coordinates (range -1 to 1 in both dimensions)
	return get_default_coordinate_frame().scr(*pargs)
	
def shift(pix,pos=None):
	if pos==None: pos = scr(0,0)
	return (pos[0]+pix[0], pos[1]+pix[1])
def up(npixels,pos=None):
	return shift((0,+npixels),pos)
def down(npixels,pos=None):
	return shift((0,-npixels),pos)
def left(npixels,pos=None):
	return shift((-npixels,0),pos)
def right(npixels,pos=None):
	return shift((+npixels,0),pos)
	