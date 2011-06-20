__all__ = ['nosqueeze', 'VirtualNoSqueezeModule']

class nosqueeze(object):
	"""
	This is a wrapper class which takes a numpy function (or numpy-like
	function) and returns a potentially less annoying version of it.
	
	This applies to all functions and object methods which operate on a
	numpy array (or something that can be turned into one), take an optional
	keyword argument called 'axis' and, if axis is supplied, return a numpy
	array in which the dimension specified by axis has been squeezed out.
	Examples in the numpy namespace are mean, median, std, ptp, max, min....
	
	The wrapped version has one difference: IT DOESN'T SQUEEZE.
	This makes it easier, for example, to subtract the
	mean-along-the-nth-dimension from each slice of a multi-dimensional
	array (for 2 or fewer dimensions, the numpy.matrix class has methods
	which behave this way already).
	
	The following are three equivalent implementations of the above example:
		x = numpy.array([  [ [1,2], [3,4] ],  [ [5,6], [7,8] ]  ])
		xc = x - nosqueeze(x.mean)(axis=1)
		xc = x - nosqueeze(numpy.mean)(x, axis=1)
		xc = x - numpy.nosqueeze.mean(x, axis=1)
	
	The third version is made possible by the VirtualNoSqueezeModule class,
	which this module automatically attaches to numpy and scipy (if
	available) when imported for the first time.
	"""###
	def __init__(self, func):
		self.func = func
		self.__doc__ = getattr(func, '__doc__', None)
	def __repr__(self):
		return "<%s instance at 0x%08X wrapped around %s>" % (self.__class__.__name__,id(self),repr(self.func).rstrip('>').lstrip('<'))
	def __call__(self, *pargs, **kwargs):
		xout = self.func(*pargs, **kwargs)
		axis = kwargs.get('axis')
		if axis != None:
			if hasattr(self.func, '__self__'): xin = self.func.__self__
			else: xin = pargs[0]
			if not isinstance(xout, numpy.ndarray): xout = numpy.asarray(xout)
			if not isinstance(xin, numpy.ndarray): xin = numpy.asarray(xin)
			shape = list(xin.shape)
			if xin.size == xout.size * shape[axis] and xin.ndim == xout.ndim + 1:
				shape[axis] = 1
				xout.shape = shape
		return xout

class VirtualNoSqueezeModule(object):
	"""
	This class is a "virtual module" containing arbitrary functions, each
	wrapped by the nosqueeze class. One example of this occurs automatically
	when you import the file in which VirtualNoSqueezeModule is defined:
		numpy.nosqueeze = VirtualNoSqueezeModule(numpy)
	Thereafter you can smoothly call the nosqueeze version of any numpy
	function that has the 'axis' keyword:
		x = [  [ [1,2], [3,4] ],  [ [5,6], [7,8] ]  ]
		xc = x - numpy.nosqueeze.mean(x, axis=1)
	Note that some of the functions in the virtual module may be false
	positives, i.e. functions that have the 'axis' keyword but which do
	not squeeze out that dimension: numpy.cumsum is one example. Calling
	these should be safe (but misleading for maintainers).
	"""###
	def __init__(self, module):
		for x in module.__dict__.values():
			if callable(x):
				f = getattr(x, 'func_code', None)
				if f != None and 'axis' in f.co_varnames:
					if not hasattr(self, x.__name__):
						setattr(self, x.__name__, nosqueeze(x))
try: import numpy
except: pass
else: numpy.nosqueeze = VirtualNoSqueezeModule(numpy)

try: import scipy
except: pass
else: scipy.nosqueeze = VirtualNoSqueezeModule(scipy)

