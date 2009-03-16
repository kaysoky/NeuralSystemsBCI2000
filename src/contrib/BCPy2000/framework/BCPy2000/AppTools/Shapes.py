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
__all__ = ['PolygonTexture']

from Boxes import box

import Image, ImageDraw, VisionEgg.Textures
from VisionEgg.Textures import gl

def PolygonTexture(frame, vertices=((0.0,1.0), (1.0,1.0), (0.5,0.0)), color=(0,0,0), anchor=None, position=None, **kwargs):
	if isinstance(frame, (list,tuple)):
		if len(frame)==2: frame = [0,0] + list(frame)
		frame = box(rect=frame)
	size = tuple(map(int, frame.size))
	if anchor==None: anchor = frame.anchor
	if position==None: position = frame.position

	# map from lowerleft-upward normalized coordinates to upperleft-downward pixel coordinates
	mapvertex = lambda x: (size[0]*x[0], size[1]-size[1]*x[1])
	vertices = map(mapvertex, vertices)
	
	# map colours from normalized to 8-bit integer, and add alpha channel if absent
	color = map(lambda x:int(round(x*255.0)), color)
	if len(color)==3: color += [255]

	canvas = Image.new("RGBA", size, (0,0,0,0))
	draw = ImageDraw.Draw(canvas)
	draw.polygon(vertices, fill=tuple(color))

	vestim = VisionEgg.Textures.TextureStimulus(
		texture=VisionEgg.Textures.Texture(canvas),
		size=size, anchor=anchor, position=position, 
		mipmaps_enabled=0, internal_format=gl.GL_RGBA, texture_min_filter=gl.GL_LINEAR,
		**kwargs
	)
	return vestim
