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
	'svd', 'csp', 'csp_itfe',
	'lda', 'running_mean', 'running_cov', 'dprime',
	'asmatrix', 'mad', 'loadmat', 'savemat', 'sdict',
	'all_pairs', 'one_versus_rest', 'infer_classes',
	'logistic', 'invlogistic', 
]
import numpy
from SigTools.NumTools import mad, loadmat, savemat, summarize, sdict

def asmatrix(x, dtype=numpy.float64, copy=False):
	"""
	Very similar to numpy.asmatrix, except that dtype is either
	specified explicitly or assumed to be float64 (where numpy would
	guess)  and lists/tuples/1-D arrays become n-by-1 matrices
	instead of numpy's bizarre default of 1-by-n.
	"""###
	if isinstance(x, numpy.matrix):
		if copy: return x.copy()
		else: return x
	if hasattr(x,'shape'):
		shape = x.shape
	else:
		x = numpy.array(x, dtype=dtype)
		shape = x.shape
	x = numpy.matrix(x, dtype=dtype, copy=copy)
	if len(shape)<2: x = x.T
	return x

class svd(object):
	"""
	d = svd(A)   where A is m-by-n
	
	The following are computed immediately, using the singular value
	decomposition  A = d.U * numpy.diag(d.s) * d.V.H
	
	             d.s: the singular values of A in decreasing order
	          d.rank: estimated rank r of A, = #{singular values > tol},
	                  where tol defaults to max(m,n)*eps*max(d.s)
	          d.cond: estimated condition number of A, = max(d.s)/min(d.s)
	             d.U: m by r  orthonormal basis for the column space of A
	      d.leftnull: m by min(m,n)-r  the discarded columns of U
	             d.V: n by r  orthonormal basis for the row space of A
	          d.null: n by min(m,n)-r  the discarded columns of V
	      d.original: a copy of A, unless keepcopy is set to False
	      
	The following can then be computed cheaply. They are computed on demand
	the first time they are requested, and then cached:
	
	          d.pinv: pseudo-inverse of A
	         d.sqrtm: X such that  X*X.H = (U*S*U.H) and X.H*X = (V*S*V.H)
	                  (for symmetric A, therefore, X is the matrix-square-root)
	        d.isqrtm: the inverse of d.sqrtm, when A is invertible:  i.e. X
	                  such that X*X.H = (U*S*U.H).I  and  X.H*X = (V*S*V.H).I
	d.reconstruction: reconstruction of A, without using the discarded
	                  columns of d.U and d.V
	"""###
	def __init__(self, A, tol=None, keepcopy=True):
		A = asmatrix(A, copy=keepcopy)
		(U, s, Vh) = numpy.linalg.svd(A, full_matrices=False, compute_uv=True)
		if tol==None: tol = max(A.shape) * numpy.finfo(A.dtype).eps * max(s)
		r = numpy.sum(s > tol)
		if keepcopy: self.original = A
		self.rank = r
		smin,smax = min(s),max(s)
		if smin: self.cond = smax/smin
		else: self.cond = numpy.inf
		self.U = U[:,:r]          # columns of U are an orthonormal basis for the column space of A
		self.s = s[:r]
		self.V = Vh[:r,:].H       # columns of V are an orthonormal basis for the row space of A
		self.null = Vh[r:,:].H
		self.leftnull = U[:,r:]
	def __getattr__(self, key):
		if key == 'pinv':           self.__dict__[key] = self.V * numpy.diag(1.0/self.s)   * self.U.H
		if key == 'sqrtm':          self.__dict__[key] = self.U * numpy.diag(self.s**0.5)  * self.V.H          # X*X.H = (U*S*U.H),   X.H*X = (V*S*V.H)
		if key == 'isqrtm':         self.__dict__[key] = self.U * numpy.diag(self.s**-0.5) * self.V.H          # X*X.H = (U*S*U.H).I, X.H*X = (V*S*V.H).I  for invertible matrices
		if key == 'reconstruction': self.__dict__[key] = self.U * numpy.diag(self.s)       * self.V.H
		if not hasattr(self, key): raise AttributeError, key
		return self.__dict__.get(key)
	def _getAttributeNames(self):
		return ['pinv', 'sqrtm', 'isqrtm', 'reconstruction']
	def __repr__(self):
		s = "<%s.%s instance at 0x%08X>" % (self.__class__.__module__,self.__class__.__name__,id(self))
		s = '\n'.join([s] + map(lambda key: "    %s: % 3d by % 3d" % (tuple([key]+list(getattr(self,key).shape))),  ['U', 'V']))
		return s

def all_pairs(classes):
	"""
	for neg,pos in all_pairs(5):
		print neg,"versus",pos
	
	You get the idea. The input may alternatively be a sequence of class
	identifiers.	
	"""###
	if isinstance(classes, (float,int)): classes = range(1,int(classes)+1)
	classes = sorted(tuple(set(classes)))
	pairs = []
	for x in xrange(len(classes)):
		for y in xrange(x):
			pairs.append((classes[y], classes[x]))
	return tuple(pairs)

def one_versus_rest(classes):
	"""
	for neg,pos in one_versus_rest(5):
		print neg,"versus",pos
	
	You get the idea. The convention is for the "rest" to come out first
	(to be labelled as -1 in the binary sub-problem) and the "one" to come
	out second (to be labelled as +1).
	
	The input may alternatively be a sequence of class identifiers.	
	"""###
	if isinstance(classes, (float,int)): classes = range(1,int(classes)+1)
	classes = set(classes)
	standardize = lambda x: tuple(sorted(tuple(x))) # because python sorts set((-1,1)) the wrong way round for some inexplicable reason
	if len(classes) == 2: return (standardize(classes),)
	rest = lambda x: standardize(classes.difference((x,)))
	pairs = [(rest(one),one) for one in standardize(classes)]
	return tuple(pairs)
		
def infer_classes(cc, types=None):
	"""
	classes,data = infer_classes(data)
	
	If data is a 2-element list or tuple, assign the classes (-1, +1).
	Otherwise, if data is an n-element list or tuple, return the classes
	1 through n.
	
	If data is a dictionary, the classes will be inferred from the keys,
	and the classes and data delivered in a standardized order.  If all the
	keys are scalar, they will simply be returned sorted in ascending order.
	If any are sequences, then all the keys will be made into sequences,
	sorted within themselves and then delivered in an order that is sorted
	first by decreasing length, then by value.  So, for example, the input
	
		{3:'one',  (2,1): 'rest'}
	
	will yield  classes=(  (1,2) , (3,)  )  and  data=( 'rest' , 'one' ).
	The sorting-by-decreasing-length ensures that, in a one-versus-rest
	problem, the "rest" will always come out first (to be mapped to -1) and
	the "one" will come out second (to be mapped to +1).
	"""###
	
	if len(cc) == 1 and isinstance(cc[0], (list,tuple,set,dict)): cc = cc[0]
	if len(cc) == 2: classes = (-1, +1)
	else: classes = tuple(range(1,len(cc)+1))
	isseq = lambda x: isinstance(x,(tuple,list,set))
	tuplify = lambda x: isseq(x) and tuple(x) or (x,)
	if isinstance(cc, dict):
		k = cc.keys()
		v = cc.values()
		if True in map(isseq, k):
			k = [tuple(set(tuplify(x))) for x in k]
			keylencmp = lambda x,y: cmp( (-len(x[0]),)+x[0],(-len(y[0]),)+y[0] )
			classes, cc = zip(*sorted(zip(k,v),cmp=keylencmp))
		else:
			classes, cc = zip(*sorted(cc.items()))
	if isinstance(types, list): types = tuple(types)
	if types != None and False in map(lambda c:isinstance(c,types) , cc):
		raise TypeError, 'inputs must be of one of the following types: '+repr(types)
	return classes, cc

class CSPError(Exception): pass
class csp(object):
	def __init__(self, classcov, globalcov=None, whichclass=1, normalize=False):
		"""
		Implements the CSP algorithm of Koles (1991).
		
		classcov is the class covariance matrix.  Alternatively, it is a
		sequence or dict of class-covariance matrices, in which case the
		class labels are inferred using infer_classes(), and the matrix
		corresponding to whichclass is used.
		
		globalcov is the global covariance. If omitted, then classcov must
		be a sequence or dict containing information about all relevant
		classes.  Then globalcov is computed as an equal weighting between
		the class of interest (whichclass) and the mean of other classes.
		
		normalize is a boolean option specifying whether to normalize
		covariance matrices by their trace before use.
		
		The output object c has the following attributes:
		
		    c.classes     : the sequence of inferred classes
		    c.whichclass  : the class of interest
		    c.opts        : dict containing options (normalize)
		    c.classcov    : the class covariance matrix of interest (after
		                    normalization, if any)
		    c.globalcov   : the global covariance matrix used (after
		                    normalization, if any)
		    c.whitening   : symmetric whitening or spherizing matrix
		    c.rotation    : rotation directions (one in each column)
		    c.filters     : spatial filters (one in each column)
		    c.patterns    : spatial patterns (one in each column)
		    c.eigenvalues : eigenvalue corresponding to each column
		    
		The best() method is useful for selecting based on eigenvalue.
		"""###
		if not isinstance(classcov, (list,tuple,dict)): classcov = {whichclass:classcov}
		self.classes,classcov = infer_classes(classcov)
		if not whichclass in self.classes: raise CSPError, 'no information about class'+str(whichclass)

		for i in range(len(classcov)):
			if isinstance(classcov[i],running_mean): classcov[i] = classcov[i].m
			classcov[i] = numpy.matrix(classcov[i]) # copies, and ensures matrixhood
			if normalize: classcov[i] /= classcov[i].trace()			
		d = dict(zip(self.classes,classcov))
		if len(self.classes)==1: self.classes = (-1,+1)
		self.whichclass = whichclass
		self.opts = {'normalize':normalize}
		self.classcov = d.pop(whichclass)
		
		if globalcov==None:
			if len(d) == 0: raise CSPError, 'too few matrices'
			globalcov = 0.5 * self.classcov + 0.5 * sum(d.values())/len(d)
			# (equal weighting of whichclass and the rest)
		if isinstance(globalcov,running_mean): globalcov = globalcov.m
		globalcov = numpy.matrix(globalcov) # copies, and ensures matrixhood
		if normalize: globalcov /= globalcov.trace()
		self.globalcov = globalcov

		s1 = svd(self.globalcov)
		self.whitening = s1.isqrtm
		m = self.whitening.H * self.classcov/2.0 * self.whitening
		s2 = svd(m)
		self.rotation = s2.U
		self.filters = self.whitening * self.rotation
		self.patterns = self.globalcov * self.filters
		for j in range(self.patterns.shape[1]):
			pattern = self.patterns[:,j]
			if numpy.max(numpy.abs(pattern)) == -numpy.min(pattern):
				pattern *= -1.0
				self.filters[:,j] *= -1.0
				self.rotation[:,j] *= -1.0
		self.eigenvalues = s2.s

	def best(self, n, m=None):
		"""
		c.best(n)  or  c.best([n]) returns a list of indices to the best
		n eigenvalues in the sense of absolute deviation from 0.5.
		
		c.best(n, m) or c.best([n,m])  returns a list of indices to the
		best n eigenvalues from the high end of the spectrum and the
		best m eigenvalues from the low end.
		
		Example:
		    ind = c.best(3, 3)
		    filt = c.filters[:, ind]
		    pat = c.patterns[:, ind]
		    eig = c.eigenvalues[:, ind]
		"""###
		n = numpy.array(n,copy=False).ravel().tolist()
		if len(n) == 1 and m != None:
			n += numpy.array(m,copy=False).ravel().tolist()
		if len(n) == 1:
			n += [0]
			d = numpy.abs(self.eigenvalues - 0.5)
		elif len(n) == 2:
			d = self.eigenvalues
		else: raise CSPError, 'expected either 1 or 2 numbers'
		pp = [(d[i], i) for i in xrange(len(d))]
		pp = [p[1] for p in sorted(pp, reverse=True)]
		out = pp[:n[0]]
		if n[1]: out += pp[-n[1]:]
		return out

def csp_itfe(filters, classcov=None, globalcov=None, classprob=None):
	"""
	For a set of spatial <filters> expressed one-per-column, a dict or
	sequence of <classcov> objects ( as supplied to csp() ) return the
	Information-Theoretic Feature Extraction criterion for CSP defined in:
	    Grosse-Wentrup and Buss (2008)
	      IEEE Transactions on Biomedical Engineering 55(8), 1991-2000
	
	Optionally, a list/numpy-array/dict of class probabilities may be
	supplied in <classprob>.
	"""###
	if isinstance(filters, csp):
		c = filters
		filters = c.filters
		if classcov == None: classcov = (c.classcov,)
		if globalcov == None: globalcov = c.globalcov
	if classcov==None: raise CSPError, 'class covariance matrices not supplied'
	
	if isinstance(globalcov, running_cov): globalcov = globalcov.C
	classes,classcov = infer_classes(classcov, [numpy.ndarray, running_cov])
	M = len(classes)
	if M == 1:
		if globalcov==None: raise CSPError, 'not enough covariance matrices'
		classcov = classcov[0]
		if isinstance(classcov, running_cov): classcov = classcov.C
		classcov = (2.0*globalcov-classcov, classcov)
		classes = (-1,1)
		M = 2
	import IPython;IPython.Debugger.Tracer()()
	if classprob == None:
		classprob = numpy.asmatrix(numpy.ones((M,1)))/M
	else:
		if isinstance(classprob, numpy.ndarray):
			classprob = classprob.flatten().tolist()
		cl,clp = infer_classes(classprob)
		if len(cl) != len(classes) or (isinstance(classprob,dict) and cl != classes):
			raise CSPError, 'class labels are inconsistent between classcov and classprob inputs'
		classprob = numpy.matrix(clp).T
	W = numpy.asmatrix(filters)
	nfilt = W.shape[1]
	proj = numpy.asmatrix(numpy.zeros((nfilt,M)))
	default_globalcov = 0.0
	classcov = list(classcov)
	for j in xrange(M):
		if isinstance(classcov[j], running_cov): classcov[j] = classcov[j].C
		classcov[j] = numpy.matrix(classcov[j])
		C = classcov[j] / float(M)
		default_globalcov = default_globalcov + C
		proj[:, j].flat = numpy.diag(W.H * C * W)
	if globalcov == None: globalcov = default_globalcov
	nrm = numpy.diag(W.H * globalcov * W)
	nrm.shape += (1,)
	proj = proj / nrm
	v = -numpy.log(numpy.power(proj, 0.5))*classprob - numpy.power((numpy.power(proj,2.0)-1.0)*classprob, 2.0) * 3.0/16.0
	return v
	
class running_mean(object):
	"""
	An object that keeps track of the mean and variance of a series
	of values x presented online. Each x may be a scalar value or a
	numpy.array.
	
	Exemplars are added to object r by the += operator:
		
		r = running_mean()
		r += x

	The object r has the following attributes:
			
	    r.n          : number of samples so far
	    r.m          : mean of x so far (same shape as incoming x)
	    r.v          : variance of x so far, normalized by r.n (same
	                   shape as incoming x)		
	    r.v_unbiased : a virtual attribute which returns the variance
	                   normalized by (r.n - 1.0) instead of by r.n
	                   
	If r.persistence=1.0, then all previous samples are "remembered"
	and each incoming exemplar counts as r.increment number of new
	samples (the increment may be measured in any units you like -
	seconds, for example). If r.persistence < 1.0,  then an
	exponential forgetting factor of 1.0-r.persistence is used, and
	although self.increment is added to s.n,  s.n is not used in
	estimation.
	
	The reset() method zeroes everything.
	"""###
	def __init__(self, persistence=1.0, increment=1.0):
		"""
		The persistence and increment arguments initialize the
		self.persistence and self.increment attributes.
		"""###
		self.increment = float(increment)
		self.persistence = float(persistence)
		self.reset()
	def reset(self):
		self.n = 0.0
		self.m = 0.0
		self.v = 0.0
	def __iadd__(self, x):
		persistence = self.persistence
		if persistence == 1.0: persistence = self.n / (self.n + self.increment)
		if self.n == 0.0: persistence = 0.0
		delta = x - self.m
		self.n += self.increment
		self.m = persistence * self.m  +  (1.0 - persistence) * x
		self.v = persistence * self.v  +  (1.0 - persistence) * persistence * numpy.multiply(delta, numpy.conj(delta))
		return self
	def __getattr__(self, key):
		if key == 'v_unbiased': return self.v * (self.n / (self.n - self.increment))
		raise AttributeError, key
	def _getAttributeNames(self):
		return ['v_unbiased']
	def run(self, x, axis=-1, reset=False):
		"""
		Test the running_mean object by adding <x> one sample at a time,
		where samples are slices concatenated along the specified <axis>.
		
		If <reset> is passed as True, the object is reset first.
		
			running_mean().run(x, axis=0).v
		
		should be the same as numpy.var(x, axis=0)
		"""###
		if reset: self.reset()
		x = numpy.array(x, copy=False)
		x = x.view()
		if axis < 0: axis += len(x.shape)
		x.shape = tuple(list(x.shape) + [1]*(axis+1-len(x.shape)))
		sub = [slice(None)] * len(x.shape)
		for i in range(x.shape[axis]): sub[axis] = i; self += x[sub]
		return self

class running_cov(object):
	"""
	Similar to running_mean, except that the x.flatten()ed content
	of each  incoming exemplar x is used, instead of x in its original
	shape. The additional attributes r.C and r.C_unbiased keep track
	of the full covariance matrix between the elements of x.
	"""###
	def __init__(self, persistence=1.0, increment=1.0):
		self.increment = float(increment)
		self.persistence = float(persistence)
		self.reset()
	def reset(self):
		self.n = 0.0
		self.m = 0.0
		self.C = 0.0
	def __iadd__(self, x):
		persistence = self.persistence
		if persistence == 1.0: persistence = self.n / (self.n + self.increment)
		if self.n == 0.0: persistence = 0.0
		x = numpy.array(x, copy=False).flatten()
		delta = asmatrix(x - self.m)
		self.n += self.increment
		self.m = persistence * self.m  +  (1.0 - persistence) * x;
		self.C = persistence * self.C  +  (1.0 - persistence) * persistence*delta*delta.H;
		return self
	def __getattr__(self, key):
		if key == 'v':          return numpy.diag(self.C)
		if key == 'v_unbiased': return self.v * (self.n / (self.n - self.increment))
		if key == 'C_unbiased': return self.C * (self.n / (self.n - self.increment))
		raise AttributeError, key
	def _getAttributeNames(self):
		return ['v', 'v_unbiased', 'C_unbiased']
	def run(self, x, axis=-1, reset=False):
		"""
		Test the running_cov object by adding <x> one sample at a time,
		where samples are slices concatenated along the specified <axis>.
		
		If <reset> is passed as True, the object is reset first.
		
			running_cov().run(x, axis=1).C_unbiased
		
		should be the same as numpy.cov(x)
		"""###
		if reset: self.reset()
		x = numpy.array(x, copy=False)
		x = x.view()
		if axis < 0: axis += len(x.shape)
		x.shape = tuple(list(x.shape) + [1]*(axis+1-len(x.shape)))
		sub = [slice(None)] * len(x.shape)
		for i in range(x.shape[axis]): sub[axis] = i; self += x[sub]
		return self
	def plot(self, *pargs, **kwargs):
		"""
		Works only for an object that has accumulated information about
		two-dimensional inputs. Plots an ellipse centred on the computed
		mean, indicating the shape of the covariance of the distribution
		of x. The size of the ellipse is specified by optional keyword
		argument nstd=2.0 (any other arguments are passed through to plot).
		"""###
		nstd = kwargs.pop('nstd', 2.0)
		r = numpy.linspace(0, 2*numpy.pi, 100)
		x = asmatrix(self.m).A + nstd*(svd(self.C).sqrtm * numpy.matrix([numpy.cos(r), numpy.sin(r)])).A
		pylab = load_pylab()
		pylab.plot(x[0],x[1],*pargs,**kwargs)
		
def dprime(*cc):
	"""
	Compute the dprime value (signed square root of the Fisher score)
	between two running_mean or running_cov objects which correspond
	to two different classes.
	"""###
	classes,cc = infer_classes(cc, [running_mean,running_cov])
	if len(cc) != 2: raise TypeError, 'expected two inputs'
	return (cc[1].m - cc[0].m) / numpy.sqrt(cc[1].v + cc[0].v)

class LDAError(Exception): pass
class lda(object):
	"""
	Fisher's linear discriminant analysis.  Example:
	
	    f = lda(ridge=0.1)
	    f.solve(rneg, rpos)  # rneg and rpos are running_cov objects
	                         # corresponding to the negative and positive
	                         # classes: their .m  and .C contain the
	                         # features' means and covariances respectively.
	    rneg += xneg
	    rpos += xpos
	
	    f.rebias(rneg, rpos) # don't re-solve, but re-bias according to the
	                         # updated estimates.
		
		f.predict(xtest)     # xtest is a sequence of features for a single
		                     # exemplar, or a features-by-exemplars array.
		
	f.w contains the weights
	f.b contains the bias
	"""###
	def __init__(self, ridge=0.0):
		"""
		Initialize the object's self.ridge attribute, indicating
		the amount of L2 regularization.  The self.ridge parameter is
		scaled by the mean diagonal element of the between-class
		covariance matrix, then added to the diagonal.
		"""###
		self.ridge = ridge
		
	def solve(self, *cc):
		"""
		f.solve(rneg, rpos)  # rneg and rpos are running_cov objects
		f.solve(d)           # the running_cov objects are in a sequence
		                     # or dict d (infer_classes is called).
		"""###
		self.classes,cc = infer_classes(cc, [running_cov])
		if len(cc) < 2: raise LDAError, 'need at least two classes'
		if len(cc) > 2: raise LDAError, 'multiclass LDA not supported'
		C = sum([c.C for c in cc]) # say that fast three times
		if self.ridge: C += numpy.eye(C.shape[0]) * self.ridge * C.diagonal().mean()
		self.w = numpy.linalg.solve(C, cc[1].m - cc[0].m)
		self.b = 0.0
		self.rebias(cc)
		return self
		
	def rebias(self, *cc):
		"""
		f.rebias(rneg, rpos)  # rneg and rpos are running_cov objects
		f.rebias(d)           # the running_cov objects are in a sequence
		                      # or dict d (infer_classes is called).
		"""###
		classes,cc = infer_classes(cc, [running_cov])
		m = sum(map(lambda c:c.m, cc)) / float(len(cc))
		self.b = self.b - self.predict(m)
		
	def predict(self, x):
		"""
		Input is a sequence of features for a single exemplar, or a
		features-by-exemplars array. Output is a real-valued decision
		value per exemplar.
		"""###
		return asmatrix(self.w).H* asmatrix(x) + self.b

	def plot(self, *cc):
		classes,cc = infer_classes(cc, [running_cov])
		m = sum(map(lambda c:c.m, cc)) / float(len(cc))
		cc[0].plot('b-')
		cc[1].plot('r-')
		db = [-self.w[1], self.w[0]]
		pylab = load_pylab()
		xl = list(pylab.gca().get_xlim())
		yl = list(pylab.gca().get_ylim())
		pylab.plot([m[0]-2*db[0],m[0]+2*db[0]], [m[1]-2*db[1],m[1]+2*db[1]], 'k-')
		pylab.gca().set_xlim(xl)
		pylab.gca().set_ylim(yl)

class ldatest(object):
	def __init__(self,x1=None,x2=None,ridge=0.0):
		if x1==None: x1 = numpy.random.randn(2,1)+(numpy.matrix(numpy.random.rand(2,2))*numpy.matrix(numpy.random.randn(2,1000))).A
		if x2==None: x2 = numpy.random.randn(2,1)+(numpy.matrix(numpy.random.rand(2,2))*numpy.matrix(numpy.random.randn(2,1000))).A
		self.x1 = x1
		self.x2 = x2
		self.cc = [running_cov().run(x,1) for x in [self.x1,self.x2]]
		self.s = lda(ridge=ridge).solve(self.cc)

	def plot(self):
		pylab = load_pylab()
		pylab.clf()
		pylab.plot(self.x1[0],self.x1[1],'bx')
		pylab.plot(self.x2[0],self.x2[1],'r+')
		self.s.plot(self.cc)
		ax = pylab.gca();
		xl,yl = ax.get_xlim(),ax.get_ylim()
		x = numpy.linspace(xl[0],xl[1],10)
		y = numpy.linspace(yl[0],yl[1],10)
		xx,yy=numpy.meshgrid(x,y)
		xy = numpy.concatenate((xx.reshape((1,xx.size)),yy.reshape((1,yy.size))),axis=0)
		z = self.s.predict(xy).reshape(xx.shape)
		z = logistic(z)
		z = numpy.array(z, order='C', dtype=numpy.float64, copy=True)
		pylab.contour(x,y,z,numpy.arange(0.1,1.0,0.1))
		pylab.draw()


def logistic(x):
	return 1.0 / (1.0 + numpy.exp(-x))

def invlogistic(p):
	return -numpy.log(1.0/p - 1.0)
	
def load_pylab():
	try:
		import matplotlib,sys
		if not 'matplotlib.backends' in sys.modules: matplotlib.interactive(True)
		import pylab
		return pylab
	except:
		print __name__, "module failed to import pylab: plotting methods will not work"


