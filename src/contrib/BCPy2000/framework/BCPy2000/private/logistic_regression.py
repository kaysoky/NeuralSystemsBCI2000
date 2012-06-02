__all__ = ['llr2class']

import numpy,scipy.optimize
from SigTools.LearningTools import predictor, class_loss, eeop, logistic

def f0(theta, x, y, C, relcost, norm, cache=None):
	if cache == None: cache = {}
	if len(cache) == 0 or cache['theta'].size != theta.size or (cache['theta'] != theta).any():
		cache['obj'],cache['dobj'],cache['theta'] = objective(theta, x, y, C, relcost, norm)
		#cache['used'] = cache.get('used', 0) + 1
	return cache['obj']
	
def f1(theta, x, y, C, relcost, norm, cache=None):
	if cache == None: cache = {}
	if len(cache) == 0 or cache['theta'].size != theta.size or (cache['theta'] != theta).any():
		cache['obj'],cache['dobj'],cache['theta'] = objective(theta, x, y, C, relcost, norm)
		#cache['used'] = cache.get('used', 0) + 1
	return cache['dobj']
		
def objective(theta, x, y, C, relcost=None, norm=2):	
	b = theta[0]
	w = theta[1:]
	obj = 0.0
	dobj = theta * 0.0
	x = x.view()
	x.shape = (x.shape[0], x.size/x.shape[0])
	x = numpy.asmatrix(x)
	y = numpy.asmatrix(y).T
	w = numpy.asmatrix(w).T

	v = x * w + b  # using numpy.matrix makes this line look nice - but it also stops arrays losing dimensionality every time we breathe on them (*#^$&@%#&$ numpy)
	dv_dw = x
	dv_db = 1
	f = 1.0 / (1.0 + numpy.exp(-v))
	
	df_dv = numpy.multiply(f, 1.0 - f)
	df_dw = numpy.multiply(df_dv, dv_dw)
	df_db = numpy.multiply(df_dv, dv_db)
		
	p = (1-y)/2 + numpy.multiply(y, f)
	dp_df = y
	dp_dw = numpy.multiply(dp_df, df_dw)
	dp_db = numpy.multiply(dp_df, df_db)
		
	nlp = -numpy.log(p)
	dnlp_dp = -1.0 / p
	dnlp_dw = numpy.multiply(dnlp_dp, dp_dw)
	dnlp_db = numpy.multiply(dnlp_dp, dp_db)
	
	if relcost != None:
		neg = y.flat < 0
		pos = y.flat > 0
		nlp[neg] *= relcost[0]
		nlp[pos] *= relcost[1]
		dnlp_dw[neg] *= relcost[0]
		dnlp_dw[pos] *= relcost[1]
		dnlp_db[neg] *= relcost[0]
		dnlp_db[pos] *= relcost[1]

	obj  += nlp.sum()	
	dobj[0] += dnlp_db.sum(axis=0)
	dobj[1:] += dnlp_dw.sum(axis=0).flat

	dr_db = 0.0
	if isinstance(norm, (int,float)):
		a = numpy.abs(w)
		da_dw = numpy.sign(w)
		r = numpy.power(a, norm).sum()
		dr_da = norm * numpy.power(a, norm-1.0)
		dr_dw = numpy.multiply(dr_da, da_dw)
	else:
		outer_norm = 1.0
		inner_norm = 2.0
		group_terms = {}
		group_sizes = {}
		we0 = inner_norm
		we1 = we0 - 1.0
		gte0 = outer_norm / inner_norm
		gte1 = gte0 - 1.0
		r = 0.0
		w = numpy.asarray(w).ravel()
		norm = numpy.asarray(norm).ravel()
		ids = numpy.unique(norm)
		dr_dw = numpy.zeros_like(w)
		for id in ids:
			mask = norm==id
			wm = w[mask]
			siz = wm.size
			t = numpy.mean(wm ** we0)
			r += t ** gte0
			c = (gte0 * t ** gte1) * we0 / siz
			dr_dw[mask] = c * wm ** we1
			#t = numpy.sqrt(numpy.mean(wm ** 2)); r += t; dr_dw[mask] = wm / ( t * siz )  # this is the same thing but optimized specifically for inner_norm=2, outer_norm=1:  but it appears to be no faster
	
	obj += C * r
	dobj[0]  +=  C * dr_db
	dobj[1:] += (C * dr_dw).flat
	
	return obj,dobj,theta
		
class llr2class(predictor):
	"""
	Linear logistic regression classifier, using L1-, L2- or any other L-norm regularization,
	optimized in the primal using scipy.optimize.fmin_cg with gradients.  Group-lasso
	regularization is also possible.
	
	<C> is the regularization penalty term. As for klr2class, a positive value of C indicates
	a relative value which will be multiplied by the mean of the norms of the (centered)
	training data points; a negative value indicates that abs(C) should be used directly
	unscaled.
	
	<relcost> may be None, a scalar (cost of mistakes on positive class relative to cost on
	negative class), a two-element sequence(cost of mistakes on [negative,positive] exemplars
	respectively), or the word "balance" to compensate exactly for the ratio of the sizes of
	the negative and positive groups.
	
	<norm> may be 1 (for L1 reg), 2 (for L2 reg) or any other positive value.  It may also
	be an array, the same size and shape as a single exemplar (and hence the same size and
	shape as the linear array of weights that will be computed) containing group ID numbers.
	In this case, a group-lasso style is used:  L1-regularization of the groupwise L2-norms.
	A feature group is the set of features that share the same corresponding ID number.
	Groups of features are weighted equally, compensating for differences in group size.	

	"""###
	def __init__(self, C=1.0, norm=2, relcost=(1.0,1.0), lossfunc=class_loss, lossfield='y'):
		predictor.__init__(self, lossfunc=lossfunc, lossfield=lossfield)
		self.hyper.C = C
		self.hyper.relcost = relcost
		self.hyper.norm = norm
		self.model.weights = None
		self.model.bias = None
		self.output.y = None
		self.output.f = None
		self.output.p = None
		
	def defaults(self): return {'C':[0.0, 0.001, 0.005, 0.05, 0.5, 0.8, 1.0]}
		
	def training(self, x, y):
		if y.size != x.shape[0]: raise ValueError('y must contain one label per data point (dimension 0 of x)')
		
		relcost = self.hyper.relcost
		if relcost == None: relcost = 1.0
		if relcost == 'balance': relcost = float(sum(y<0.0)) / float(sum(y>0.0))
		err = 'hyper.relcost must be a scalar, a two-element sequence, or the string "balance"'
		if isinstance(relcost, basestring): raise ValueError(err)
		relcost = numpy.asarray(relcost).flatten()
		if relcost.size == 1: relcost = relcost ** [-0.5, 0.5]
		if relcost.size != 2: raise ValueError(err)
		if relcost[0]==relcost[1]: relcost = None
			
		n = {}; m = {}
		x = numpy.array(x, dtype=numpy.float64, order='C')
		y = numpy.sign(y.flat)
		for i,yi in enumerate(y):
			n[yi] = n.get(yi, 0) + 1
			m[yi] = m.get(yi, 0.0) + x[i]
		for k,v in m.items(): v /= n[k]
		w = m[+1] - m[-1]
		w /= (w**2.0).sum()
		f = numpy.zeros((len(x),),dtype=numpy.float64)
		for i,xi in enumerate(x): f[i] = numpy.inner(w.flat,xi.flat)
		b = -eeop(f,y)
		w /= numpy.abs(f+b).mean()
		
		mm = x.mean(axis=0)
		expt = self.hyper.norm
		if not isinstance(expt, (int,float)): expt = 2.0
		ss = [(numpy.abs(xi)**expt).sum() for xi in (x-mm)]
		cscaling = sum(ss) / len(ss)
		C = self.hyper.C
		if C < 0: C *= -1
		else: C *= cscaling
		
		theta0 = numpy.zeros(w.size+1)
		theta0[0] = b
		theta0[1:] = w.flat
		cache = {}
		args = (x, y, C, relcost, self.hyper.norm, cache)
		#print f0(theta0, *args)
		out = scipy.optimize.fmin_cg(f0, theta0, fprime=f1, args=args, disp=False)
		theta = out
		#print f0(theta, *args)
		w.flat = theta[1:]
		b = theta[0]
		self.model.weights = w
		self.model.bias = b
		#print "shortcuts:",cache.get('used',0)
		
	def testing(self, x):
		w = self.model.weights.flat
		self.output.f = numpy.asarray([numpy.inner(w,xi.flat) for xi in x]) + self.model.bias
		self.output.y = numpy.sign(self.output.f)
		self.output.p = logistic(self.output.f)
		
