import numpy

class ShapeMismatch(Exception): pass

class kalman_filter(object):
	def __init__(self, n, A=1, C=1, Q=1, R=None, x0=None, V0=None, B=None):

		self.n = n
		self.I = numpy.asmatrix(numpy.eye(n))

		self.A = numpy.matrix(A)
		if len(self.A.flat) == 1:  self.A = numpy.multiply(self.A, self.I)

		self.C = numpy.matrix(C)
		if len(self.C.flat) == 1:  self.C = numpy.multiply(self.C, self.I)

		self.Q = numpy.matrix(Q)
		if len(self.Q.flat) == 1:  self.Q = numpy.multiply(self.Q, self.I)

		if R == None: R = self.Q * 50.0
		self.R = numpy.matrix(R)
		if len(self.R.flat) == 1:  self.R = numpy.multiply(self.R, self.I)

		if B == None:
			self.B = numpy.matrix([])
		else:
			self.B = numpy.matrix(B)
			if self.B.shape[0] != self.n: raise ShapeMismatch, 'B should have %d rows'%self.n

		self.x0 = x0
		if self.x0 != None:
			self.x0 = numpy.matrix(self.x0)
			self.x0.shape = (len(self.x0.flat),1)
		self.V0 = V0
		if self.V0 != None:
			self.V0 = numpy.matrix(self.V0)
					
		self.reset()
		numpy.matrix(0.5).I  # force blas to load
		

	def reset(self):
		self.t = 0
		self.x = None
		self.V = None
		self.VV = None
		self.loglike = 0.0
		self.prevx = None
		self.prevV = None
		
	def __iadd__(self, x):
		self.update(x)
		return self
		
	def update(self, y, u=None):
		y = numpy.asmatrix(y).view()
		n = len(y.flat)
		y.shape = (n,1)
		if n != self.n: raise ShapeMismatch, 'input dimensionality (%d) does not match setup (should be %d)'%(n,self.n)
		if self.t == 0:
			if self.x0 == None: self.prevx = y.copy()
			else:               self.prevx = self.x0
			if self.V0 == None: self.prevV = self.I.copy()
			else:               self.prevV = self.V0
		else:
			self.prevx = self.x
			self.prevV = self.V
		
		x = self.prevx
		V = self.prevV
		
		if self.t == 0:
			Vpred = V
			xpred = x
		else:
			Vpred = self.A * V * self.A.H + self.Q
			xpred = self.A * x

		if u != None:
			u = numpy.asmatrix(u).view()
			m = len(u.flat)
			u.shape = (m,1)
			if self.B.shape[1] != m: raise ShapeMismatch, '%d control variables supplied, %d expected'%(m, self.B.shape[1])
			xpred = xpred + self.B * u;
		
		e = y - self.C * xpred # error (innovation)
		S = self.C * Vpred * self.C.H + self.R
		#loglik = gaussian_prob(e, zeros(1,length(e)), S, 1);
		K = Vpred * self.C.H * S.I # Kalman gain matrix
		# If there is no observation vector, set K = zeros(ss).
		self.x  = xpred + K * e;
		self.V  = (self.I - K * self.C) * Vpred;
		self.VV = (self.I - K * self.C) * self.A * V;
		
		self.t += 1
		return self.x

	def run(self, y):
		y = numpy.asmatrix(y)
		x = y * 0.0
		self.reset()
		for t in range(y.shape[1]):
			x[:,t] = self.update(y[:,t])
		return x


class kf1d(object):
	def __init__(self, R=None, x0=None, V0=None, B=None):
		if R == None: R = 50.0
		self.R = R
		if B == None: B = 0.0
		self.B = B
		self.x0 = x0
		self.V0 = V0					
		self.reset()
		
	def reset(self):
		self.t = 0
		self.x = None
		self.V = None
		self.prevx = None
		self.prevV = None
		
	def __iadd__(self, x):
		self.update(x)
		return self
		
	def update(self, y, u=None):
		if self.t == 0:
			if self.x0 == None: self.prevx = y
			else:               self.prevx = self.x0
			if self.V0 == None: self.prevV = 1.0
			else:               self.prevV = self.V0
		else:
			self.prevx = self.x
			self.prevV = self.V		
		x,V = self.prevx,self.prevV
		xpred,Vpred = x,V
		if self.t > 0: Vpred += 1.0
		if u != None: xpred += self.B * u;		
		e = y - xpred
		K = Vpred / (Vpred + self.R)
		self.x  = xpred + K * e;
		self.V  = (1.0 - K) * Vpred;
		self.t += 1
		return self.x

	def run(self, y, R=None):
		if R != None: self.R = R
		y = numpy.asmatrix(y)
		x = y * 0.0
		self.reset()
		for t in range(y.shape[1]):
			x[:,t] = self.update(y[:,t])
		return x
