import numpy

def alogb(a, b, base=None):
	"""
	Compute log(a ** b) in a probability-friendly way.
	
	Under the hood it uses a * log(b) instead of log(a**b),
	to avoid numerical problems. But there is one edge case: 

	By convention 0 raised to the power 0 is 1. 
	So log(a**b) should be log(1), i.e. 0,  when a=b=0.
	So alogb(0,0) should be 0,  not the NaN you get from 0 * log(0)
	
	To see that the convention makes sense, work out the log-likelihood formula
	for obtaining 0 heads when you toss 0 coins.  Now do the same for a loaded
	coin.  Now do the same for a double-tailed coin.  Note that we want the
	probability to come out to 1 in all cases.
	"""###
	a = numpy.asarray(a, numpy.float64)
	b = numpy.asarray(b, numpy.float64)
	
	w = numpy.geterr()
	numpy.seterr(divide='ignore', invalid='ignore')
	c = a * numpy.log(b) # NB: throws warnings, even though behaviour is all as desired
	numpy.seterr(**w)
	if base != None: c /= numpy.log(base) # so far so hoopy

	# Now the result of the obligatory 2000 hours spent on ticklish numpy dimension-juggling to guard against edge cases
	isarray = isinstance(c, numpy.ndarray)
	if not isarray: c = numpy.asarray(c) # because scalar results cause the next line to fail (<sigh>)
	c.flat[numpy.logical_and(a.flat==0, b.flat==0)] = 0.0
	if not isarray: c = c.flat[0]
	return c

def acc2bitrate(p, N=2):
	"""
	Return an estimate, under certain particular assumptions, of the channel capacity,
	in bits, of a decision-maker whose probability of correctly identifying one of N
	classes is given by p.
	
	The "Wolpaw bit rate" definition is used:
	
	Wolpaw JR, Ramoser H, McFarland DJ, Pfurtscheller G. (1998)
	   EEG-based communication: improved accuracy by response verification.
	   IEEE Transactions on Rehabilitation Engineering 6(3):326-33.

	The assumptions are:
	    - all symbols are assumed to have equal prior probability
	    - accuracy p is assumed to be independent of which class is being detected
	    - all errors are assumed to be uninformative
	
	"""###
	p = numpy.asarray(p, numpy.float64)
	N = numpy.asarray(N, numpy.float64)
	return alogb(1, N, 2) + alogb(p, p, 2) + alogb(1-p, (1-p)/(N-1), 2)

def mutinf(true=None, predicted=None, confusion_matrix=None):
	"""
	bits = mutinf(true, predicted)
	bits = mutinf(confusion_matrix=C)
		where C = confuse(true, predicted)
	
	Actually mutinf() is symmetrical in its input arguments, but the
	order (true, predicted) is the convention established elsewhere,
	e.g. in balanced_loss()
	
	Returns the unnormalized, symmetric, mutual information (information gain in
	bits per trial):
	
		I(X,Y) = H(X) + H(Y) - H(X,Y)
			   = H(X) - H(X|Y)
			   = H(Y) - H(Y|X)
		
		  H(Y) = -sum( p(Y) * log2(p(Y)) )
		H(Y|X) = -sum( p(X) * sum ( p(Y|X) * log2(p(Y|X))  ) )
		
	"""###	
	if confusion_matrix == None:
		predicted = numpy.asarray(predicted).flatten()
		if (predicted > numpy.floor(predicted)).any(): predicted = numpy.sign(predicted)
		
		from SigTools import confuse # TODO: remove
		confusion_matrix,labels = confuse(true=true, predicted=predicted)

	c = numpy.asarray(confusion_matrix, dtype=numpy.float64)

	# using Kronegg's notation:
	#  x is input (true symbol from 1..N), y is output (predicted symbol from 1..M)
	
	N,M = c.shape # note N and M are not their usual way round
	
	pxy = c / sum(c.flat)
	px = numpy.expand_dims(pxy.sum(axis=1), 1)
	py = numpy.expand_dims(pxy.sum(axis=0), 0)
	b = alogb(pxy, pxy, 2) - alogb(pxy, px * py, 2)
	return sum(b.flat)
