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
__all__ = ['box', 'union_of_boxes']

import copy
class box(object):
	"""
	A class that groks the rectangle problem.
	It has the following attributes.  When any one of them is explicitly
	changed, the others are updated to remain consistent.
	        left : the left coordinate
	         top : the top coordinate
	       right : the right coordinate
	      bottom : the bottom coordinate
	        rect : a tuple of (self.left, self.bottom, self.right, self.top)
	       width : equal to (self.right - self.left)
	      height : equal to (self.top - self.bottom)
	        size : a tuple containing (self.width, self.height)
	           x : the horizontal position at the current anchor point
	           y : the vertical position of the current anchor point
	    position : a tuple containing (self.x, self.y)
	      anchor : one of the following strings indicating which part of the
	               rectangle has the coordinates denoted by (self.x,self.y):
	              'center'
	              'top'
	              'bottom'
	              'left'
	              'right'
	              'upperleft'  | 'upper left'  | 'topleft'     | 'top left'
	              'lowerleft'  | 'lower left'  | 'bottomleft'  | 'bottom left'
	              'upperright' | 'upper right' | 'topright'    | 'top right'
	              'lowerright' | 'lower right' | 'bottomright' | 'bottom right'
	      sticky : when the anchor point is changed, should the rectangle
	               remain where it is and (self.x,self.y) be updated to reflect
	               the new anchor point (sticky=True), or should the rectangle
	               move so that the existing (self.x,self.y) coordinates refer
	               to the new anchor point (sticky=False) ?
	    internal : another box object, specifying the internal coordinates of
	               the box. The map() method can be used to map from internal
	               to external coordinates. Default internal coordinates are
	               (0,0,1,1), i.e. bottom left is (0,0) and top right is (1,1).

	Useful methods include map(), shift(), scale(), split(), scr() and union().
	"""###
	
	def __init__(self, rect=None, **kwargs):
		"""
		Initialize a box object by supplying any of the attributes listed in
		the class documentation as named keyword arguments.  For example:
		   b = box(position=(100,100), size=(10,10), anchor='top right') 
		"""###
		if rect != None and not 'rect' in kwargs: kwargs['rect'] = rect
		self.__dict__.update({
		     'rect':None, 
			 'left':None, 'bottom':None, 'right':None, 'top':None,
			 'sticky':kwargs.pop('sticky', True), 
			 'anchor':kwargs.pop('anchor', 'center'), 
		     'x':None, 'y':None, 'position':None,
		     'width':None, 'height':None, 'size':None,
		     'internal':None,
		})
		self.rect = (-50,-50,+50,+50)
		for k in kwargs.keys(): setattr(self, k, kwargs[k])
		
	def __setattr__(self, key, val):
		class BoxError(Exception): pass
		if not key in self.__dict__.keys(): raise KeyError, 'no such attribute '+key
		if key in ['rect', 'size', 'position']: val = tuple(map(float, val))
		if key in ['width', 'height', 'x', 'y']:
			if isinstance(val, tuple) or isinstance(val, list): val = val[0]
			val = float(val)
		if key in ['sticky']: val = bool(val)
		prev_anchor = self.anchor
		self.__dict__[key] = val
		new_anchor = self.anchor
		if isinstance(new_anchor, (tuple,list)):
			p = new_anchor
		else:
			p = {
				'center':       (0.5,0.5),
				'top':          (0.5,1.0),    'bottom':       (0.5,0.0),       'left':         (0.0,0.5),    'right':        (1.0,0.5),
				'lowerleft':    (0.0,0.0),    'upperleft':    (0.0,1.0),       'lowerright':   (1.0,0.0),    'upperright':   (1.0,1.0),
				'bottomleft':   (0.0,0.0),    'topleft':      (0.0,1.0),       'bottomright':  (1.0,0.0),    'topright':     (1.0,1.0),
			}.get(new_anchor.replace(' ',''))
			if p == None:
				self.__dict__['anchor'] = prev_anchor
				raise BoxError, 'unrecognized anchor "%s"' % str(new_anchor)
		if key in ['left','bottom','right','top']:
			self.__dict__['rect'] = (self.left,self.bottom,self.right,self.top)
			key = 'rect'; val = self.rect
		if key in ['rect']:
			self.__dict__['left'],self.__dict__['bottom'],self.__dict__['right'],self.__dict__['top'] = self.rect
			self.__dict__['width']  = self.right - self.left
			self.__dict__['height'] = self.top - self.bottom
		if key in ['size']: self.__dict__['width'], self.__dict__['height'] = self.size
		if key in ['rect'] or (key=='anchor' and self.sticky):
			self.__dict__['x'] = self.left + self.width  * p[0]
			self.__dict__['y'] = self.bottom + self.height * p[1]
		if key in ['position']:
			self.__dict__['x'], self.__dict__['y'] = self.position
		if key in ['x', 'y','rect'] or (key=='anchor' and self.sticky):
			self.__dict__['position'] = (self.x,self.y)
		if key in ['width', 'height','rect']:
			self.__dict__['size'] = (self.width,self.height)
		if key in ['position', 'size', 'x', 'y', 'width', 'height'] or (key=='anchor' and not self.sticky):
			self.__dict__['rect'] = (self.x-self.width*p[0], self.y-self.height*p[1], self.x+self.width*(1.0-p[0]), self.y+self.height*(1.0-p[1]))
			self.__dict__['left'],self.__dict__['bottom'],self.__dict__['right'],self.__dict__['top'] = self.rect
		if isinstance(self.__dict__['internal'], (tuple, list)):
			self.__dict__['internal'] = box(rect=self.__dict__['internal'])
			
	def __repr__(self):
		s = "<%s.%s instance at 0x%08X>" % (self.__class__.__module__,self.__class__.__name__,id(self))
		s += "\n    " + "rectangle " + repr(self.rect)
		s += "\n    " + "size %g x %g, %s anchor point at (%g , %g), %s" % (self.width, self.height, str(self.anchor), self.x, self.y, (self.sticky and "sticky" or "non-sticky"))
		if self.internal != None:
			s += "\n    " + "internal coordinates X [%+g %+g] by Y [%+g %+g]" % (self.internal.left,self.internal.right,self.internal.bottom,self.internal.top)
		return s
		
	def shift(self, xy=None, x=None, y=None, proportional=False):
		"""
		Shift a box to the right by an amount equal to xy[0], if supplied,
		or x otherwise. Shift it upwards by xy[1], if supplied, or by y
		otherwise.
		
		If proportional is passed as True, then the horizontal and vertical
		shifts are interpreted as being expressed as multiples of the box's
		original width and height, respectively.  Thus,
		    b.shift(x=0.5, proportional=True)
		means "shift box b to the right by a distance equal to half its width".
		"""###
		if xy != None:
			if not isinstance(xy,tuple) and not isinstance(xy,list): xy = (xy,0)
			x,y = xy
		pos = list(self.position)
		if x != None:
			if proportional: x *= self.width
			pos[0] += x
		if y != None:
			if proportional: y *= self.height
			pos[1] += y
		self.position = pos

	def scale(self, xy=None, x=None, y=None):
		"""
		Scale a box's width by a factor equal to xy[0], if supplied, or x
		otherwise. Scale its height by xy[1], if supplied, or by y otherwise.
		"""###
		if xy != None:
			if not isinstance(xy,tuple) and not isinstance(xy,list): xy = (xy,xy)
			x,y = xy
		size = list(self.size)
		if x != None: size[0] *= x
		if y != None: size[1] *= y
		self.size = size

			
	def union(self, other):
		"""
		Return a box which encloses two boxes: self, and the other
		box specified.  The function union_of_boxes will do the
		same for an arbitrary number of boxes.
		"""###
		r = (
		      min(self.rect[0], other.rect[0]),
		      min(self.rect[1], other.rect[1]),
		      max(self.rect[2], other.rect[2]),
		      max(self.rect[3], other.rect[3]),
		     )
		return box(rect=r,sticky=self.sticky,anchor=self.anchor)
	
	def copy(self):
		return copy.deepcopy(self)

	def map(self, val=None, attr='position', box=None, src=None):
		"""
		Maps val from a box's internal to its external coordinates.
		attr specifies what kind of measurement val is: a 'width'
		for example will be mapped slightly differently from an 'x'.
		Valid values for attr are 'left', 'right', 'top', 'bottom',
		'rect', 'size', 'width', 'height', 'position', 'x' and 'y'.
		
		If another box object (argument 'box') is supplied, the
		appropriate attribute (named by argument 'attr') of that
		second box is updated using the result. If val is not
		supplied, the specified attribute of box is also used as
		input. For example:
		    inner = box(size=(0.5,0.5), position=(0.5,0.5))
		    outer.map(attr='rect', box=inner)
		creates a new box inner, centres it at the centre of box
		outer and gives it half the size of outer (assuming outer
		has the default internal coordinates (0,0,1,1))
		
		src is an optional box object specifying the source
		coordinate frame. By default, self.internal is used.
		"""###
		if val == None and box != None: val = getattr(box, attr)
		if val == None: return None
		if src==None: src = self.internal
		if src==None: src = self.__class__(rect=(0,0,1,1))
		if attr in ['size']:
			val = (self.map(val[0], 'width', src=src), self.map(val[1], 'height', src=src))
		if attr in ['position']:
			val = (self.map(val[0], 'x', src=src), self.map(val[1], 'y', src=src))
		if attr in ['rect']:
			val = (self.map(val[0], 'left', src=src), self.map(val[1], 'bottom', src=src), self.map(val[2], 'right', src=src), self.map(val[3], 'top', src=src))
		if attr in ['left', 'right', 'x']: val = self.left   + self.map(float(val) - src.left,   'width',  src=src)
		if attr in ['bottom', 'top', 'y']: val = self.bottom + self.map(float(val) - src.bottom, 'height', src=src)
		if attr in ['width']:  val = float(val) * self.width / src.width
		if attr in ['height']: val = float(val) * self.height / src.height
		if box != None: setattr(box, attr, val)
		return val
	
	def split(self, x=None, y=None):
		"""
		Return a tuple of box objects, resulting from splitting the current
		box horizontally at the specified x coordinate, or vertically at the
		specified y coordinate (expressed in terms of the box's own internal
		coordinates).  If both x and y are supplied, return a tuple of tuples
		of boxes, resulting from a vertical and then a horizontal split.
		"""###
		b = None
		if x == None and y == None:
			if self.internal==None: x = 0.5
			else: x = 0.5 * (self.internal.left + self.internal.right)
		if x != None and y != None:
			return tuple(map(lambda b:b.split(x=x), self.split(y=y)))
		if x != None: 
			x = self.map(val=x, attr='x')
			b = [self.copy(), self.copy()]
			b[0].right = x; b[1].left = x
		if y != None: 
			y = self.map(val=y, attr='y')
			b = [self.copy(), self.copy()]
			b[0].bottom = y; b[1].top = y
		if b != None: b = tuple(b)
		return b
		
	def scr(self, *pargs):
		"""
		A shortcut for self.map(..., attr='position') for which
		self.scr((x,y)) and self.scr(x,y) are equivalent, and the
		default input argument is (0,0).
		"""###
		if len(pargs) == 2: x,y = pargs
		elif len(pargs) == 1: x,y = pargs[0]
		elif len(pargs) == 0: x,y = 0,0
		else: raise TypeError, 'Wrong number of arguments'
		return self.map(val=(x, y), attr='position')


def union_of_boxes(*m):
	"""
	Return a box which encloses all of the input boxes.
	"""###
	if len(m) == 1 and isinstance(m[0], (tuple,list)): m = m[0]
	if len(m) == 0: return box(rect=(0,0,0,0))
	mm = m[0].copy()
	for i in range(1,len(m)): mm.union(m[i])
	return mm
