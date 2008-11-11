#   $Id$
#   
#   This file is part of the BCPy2000 framework, a Python framework for
#   implementing modules that run on top of the BCI2000 <http://bci2000.org/>
#   platform, for the purpose of realtime biosignal processing.
# 
#   Copyright (C) 2007-8  Thomas Schreiner, Jeremy Hill
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
__all__ = [
	'whoami', 
	'isnumpyarray', 
	'disp', 'cat', 'mad', 
	'project', 'trimtrailingdims', 
	'summarize', 'loadmat', 'savemat',
	'sdict',
]

import numpy
import re as regular_expression
from sys import _getframe as getframe

def whoami(): return getframe(1).f_code.co_name

def disp(*pargs,**kwargs):
	"""
	An alternative (more Matlab-like) way to display numpy arrays that have 2
	or fewer dimensions. Use the <fmt> keyword argument to supply the printf-
	style format string to be used on each element.
	
	See also summarize()
	"""###
	fmt=kwargs.pop('fmt','% g')
	indent=kwargs.pop('indent',0)
	inputname=kwargs.pop('inputname','') # TODO: is there an equivalent of matlab's inputname call?
	if len(kwargs): raise TypeError, "%s() got an unexpected keyword argument '%s'"%(whoami(),kwargs.keys()[0])
	spc = '  ' * indent
	for a in pargs:
		if isinstance(a,list) or isinstance(a,tuple):
			for i in range(len(a)): disp(a[i],fmt=fmt,indent=indent,inputname=inputname+"[%d]"%i)
			continue
		if isinstance(a,dict):
			for i in a.keys():  disp(a[i],fmt=fmt,indent=indent,inputname=inputname+"['%s']"%i)
			continue
		if not isnumpyarray(a): print inputname + " = " + a.__repr__() + "\n"; continue
		if len(inputname): print "%s = " % inputname
		a = project(a,1)
		comp = numpy.iscomplexobj(a)
		(nrows,ncols)=a.shape
		nchars = numpy.zeros(shape=a.shape,dtype='complex')
		arstr = []
		aistr = []
		colw = 0
		stripsign = regular_expression.compile(r'^(\s*)[\+\- ]')
		for i in range(0,nrows):
			arstr.append([])
			aistr.append([])
			for j in range(0,ncols):
				val = a[i,j].real
				realstr = (fmt%val).rstrip();
				arstr[-1].append(realstr)
				imagstr = ''
				if comp:
					val = a[i,j].imag
					opstr = ' + '
					if val < 0.0: opstr = ' - '
					imagstr = (fmt%val).rstrip() + 'j'
					imagstr = opstr + stripsign.sub(r'\1', imagstr);
				aistr[-1].append(imagstr)
				nchars[i,j]=complex(len(realstr),len(imagstr))
				colw=max(colw,len(realstr),len(imagstr))
		for i in range(0,nrows):
			s = spc
			for j in range(0,ncols):
				s += arstr[i][j] + ' ' * int(colw-nchars[i,j].real)
				if comp: s += aistr[i][j] + ' ' * int(colw-nchars[i,j].imag)
				if j < ncols-1: s += ',  '
			print s
		print ""

def isnumpyarray(x):
	return isinstance(x, numpy.ndarray)
	
def cat(*pargs,**kwargs):
	"""
	cat(a1,a2,...., axis=1)
	
	Like numpy.concatenate, but makes a decent effort to project the arrays
	up into the number of dimensions required by axis, and by the highest-
	dimensional input.
	"""###
	axis=kwargs.pop('axis',1)
	if len(kwargs): raise TypeError, "%s() got an unexpected keyword argument '%s'"%(whoami(),kwargs.keys()[0])
	if len(pargs) == 1 and isinstance(pargs[0], (tuple,list)): pargs = pargs[0]
	pargs = list(pargs) # makes a copy, at least of the list container
	nd = max([axis] + [len(x.shape)-1 for x in pargs if isinstance(x,numpy.ndarray)])
	for i in range(0,len(pargs)):
		if not isinstance(pargs[i], numpy.ndarray) or nd > 1:
			pargs[i] = numpy.asarray(pargs[i],dtype='float')
		pargs[i] = project(pargs[i],nd)
	return numpy.concatenate(tuple(pargs), axis=axis)
	
def mad(a,b=None,axis=None):
	"""
	Maximum absolute difference between two arrays, or maximum absolute
	value of one array, along a certain axis (if given) or among all
	elements (if not).
	"""###
	if b != None: a = a - b
	a = numpy.abs(a)
	if axis!=None: a = project(a, axis)
	return numpy.max(a,axis=axis)

def project(a, maxdim):
	"""
	Return a view of the numpy array <a> that has at least <maxdim>+1
	dimensions.
	
	See also trimtrailingdims()
	"""###
	if isinstance(a, numpy.matrix) and maxdim > 1:
		a = numpy.asarray(a)
	else:
		a = a.view()
	a.shape += (1,) * (maxdim-len(a.shape)+1)
	return a

def trimtrailingdims(a):
	"""
	Similar to numpy.squeeze() but only removes high dimensions, like
	matlab's squeeze. Returns a view of array <a>.
	
	See also project()
	"""###
	a = numpy.asarray(a).view()
	sh = list(a.shape)
	while len(sh) and sh[-1]==1: sh.pop(-1)
	a.shape = tuple(sh)
	return a

def summarize(a):
	"""
	Returns a somewhat easier-to-swallow string representation of
	an object, when the full repr() would otherwise be too long.
	In particular, summarize the interesting atrributes of numpy
	arrays and matrices.
	
	See also the sdict class.
	"""###
	if isinstance(a, type(None)): return "None"
	
	if isinstance(a, numpy.matrix): atype = 'numpy.matrix'
	elif isinstance(a, numpy.ndarray): atype = 'numpy.array'
	else: atype = a.__class__.__name__
	
	if hasattr(a, 'shape'):
		shape = 'shape='+str(a.shape).replace(' ', '')
		nels = numpy.prod(a.shape)
	elif hasattr(a, '__len__'):
		shape = 'length='+str(len(a))
		nels = len(a)
	else:
		shape = ''
		nels = 1
	
	if isinstance(a, str) and nels < 20:
		atype += '(%s)'%repr(a)
	elif nels < 20:
		s = ','.join(str(a).split()).replace('[,','[').replace(',]',']').replace(',,',',')
		if len(s) < 20: atype += '(%s)'%s

	if isinstance(a, (float,int,str)): addr = ''
	else: addr = 'id=0x%08x' % id(a)

	
	if isinstance(a, numpy.ndarray): base = a.base
	else: base = None
	if base == None: base = ''
	else: base = 'base=0x%08x' % id(base)
	
	dtype = getattr(getattr(a, 'dtype', ''), 'name', '')
	if dtype != '': dtype = 'dtype=numpy.'+dtype
	
	try: order = a.flags.farray and "order='F'" or "order='C'"
	except: order = ''
	
	s = ', '.join(filter(len, (addr, shape, dtype, order, base)))
	s = ': '.join(filter(len, (atype, s)))
	return s	

def loadmat(filename):
	"""
	Wraps scipy.io.mio.loadmat to understand '/' as a universal
	file-separator, and to yield easier-to-inspect output (sdict
	class).
	"""###
	import os,scipy.io.mio
	if isinstance(filename,str): filename = filename.replace('/', os.path.sep)
	return sdict(scipy.io.mio.loadmat(filename, squeeze_me=False))

def savemat(filename, d, append=False):
	"""
	Wraps scipy.io.mio.savemat to understand '/' as a universal
	file-separator, to save d.__dict__ if d is not itself a dict,
	and to filter out (and warn about) any unsaveable items.
	"""###
	import os,scipy.io.mio
	if isinstance(filename,str): filename = filename.replace('/', os.path.sep)
	if not filename.lower().endswith('.mat'): filename += '.mat'
	if not isinstance(d, dict): d = d.__dict__
	
	before = set(d.keys())
	selectval = lambda x: isinstance(x, (str,int,float,numpy.ndarray)) or (isinstance(x,(tuple,list)) and not False in [isinstance(y,(float,int)) for y in x])
	selectkey = lambda x: isinstance(x, str) and not x.startswith('_')
	dd = dict(filter(lambda x: selectkey(x[0]) and selectval(x[1]), d.items()))
	gone = before.difference(dd.keys())
	for k in gone: print "warning: unable to save item  %s: %s" % (repr(k), summarize(d[k]))
	if len(dd) == 0: raise TypeError, 'nothing to save'
	scipy.io.mio.savemat(filename, dd, appendmat=append)

class sdict(dict):
	"""
	A dict subclass which uses summarize() to represent its items rather than
	the whole string representation of each one.  Items are also accessible
	in "lazy" fashion, like .this  as well as like ['this'], unless a genuine
	attribute of the same name exists.
	"""###
	def __init__(self, *pargs, **kwargs):
		if len(pargs)==1 and len(kwargs)==0 and isinstance(pargs[0], (tuple,list)):
			z = zip(xrange(len(pargs[0])), pargs[0])
			dict.__init__(self, z)
		else:
			dict.__init__(self, *pargs, **kwargs)
		
	def __repr__(self):
		s = "<%s.%s instance at 0x%08X>" % (self.__class__.__module__,self.__class__.__name__,id(self))
		k = sorted(self.keys())
		ks = ["'%s'"%str(x) for x in sorted(self.keys())]
		fmt = '% '+str(4+max([0]+map(len,ks)))+"s: %s"
		return '\n'.join([s] + [fmt%(str(x),summarize(self[x])) for x in k])
	def copy(self):
		return self.__class__(self)
	def __setattr__(self, key, value):
		if self.__dict__.has_key(key): self.__dict__[key] = value
		elif self.has_key(key): self[key] = value
		else: raise AttributeError, "'%s' object has no attribute '%s'"%(self.__class__.__name__, key)
	def __getattr__(self, key):
		if self.__dict__.has_key(key): return self.__dict__[key]
		elif self.has_key(key): return self[key]
		else: raise AttributeError, "'%s' object has no attribute '%s'"%(self.__class__.__name__, key)
	def _getAttributeNames(self):
		return self.keys()

