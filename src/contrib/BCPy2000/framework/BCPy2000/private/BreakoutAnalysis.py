__all__ = ['BreakoutDir', 'breakout', 'ClopperPearson', 'PluginGaussianApproximation', 'bincvgtest', 'robotest']

import BCI2000Tools.FileReader
import SigTools
import time
import numpy

BreakoutDir = 'c:/bci2000/3.x/data/breakout-test001'

def PluginGaussianApproximation(nSuccesses, nTrials, alpha=0.05):
	p = float(nSuccesses)/float(nTrials)
	ste = (p * (1.0-p) / float(nTrials)) ** 0.5
	alpha = numpy.asarray(alpha).flatten()
	if alpha.size == 1: alpha = numpy.r_[alpha/2.0, 1.0-alpha/2.0]
	lowerLimit = p + SigTools.invcg(min(alpha)) * ste
	upperLimit = p + SigTools.invcg(max(alpha)) * ste
	return lowerLimit,upperLimit
def ClopperPearson(nSuccesses, nTrials, alpha=0.05):
	import scipy.stats
	nFailures = nTrials - nSuccesses	
	invFCDF = lambda p,df1,df2: scipy.stats.f(df1,df2).isf(1.0 - p) 
	lowerLimit, upperLimit = 0.0, 1.0
	alpha = numpy.asarray(alpha).flatten()
	if alpha.size == 1: alpha = numpy.r_[alpha/2.0, 1.0-alpha/2.0]
	
	if nSuccesses > 0:
		df1 = nSuccesses * 2.0;
		df2 = (nFailures + 1.0) * 2.0;
		f = invFCDF(min(alpha), df1, df2)
		lowerLimit = (df1 * f) / (df1 * f + df2)
	if nFailures > 0:
		df1 = (nSuccesses + 1) * 2.0
		df2 = nFailures * 2.0
		f = invFCDF(max(alpha), df1, df2);
		upperLimit = (df1 * f) / (df1 * f + df2);
	return lowerLimit,upperLimit
def bincvgtest(P=None, N=None, R=999, alpha=0.05, method=ClopperPearson):
	if N == None: N = (5,6,7,8,9,10,12,14,16,18,20)
	if P == None: P = numpy.arange(0.05, 1.0, 0.05)
	coverage = numpy.zeros((len(N),len(P)),dtype=numpy.float64)
	for iN,n in enumerate(N):
		for ip,p in enumerate(P):
			lo,up = zip(*[method(x, n, alpha) for x in numpy.random.binomial(n, p, R)])
			c = numpy.logical_and(numpy.asarray(lo) <= p, p <= numpy.asarray(up)).sum()/(R+1.0)
			coverage[iN,ip] = c
			print n,p,c
	return SigTools.sdict({'n':N, 'p':P, 'coverage':coverage})


insane_fields = ['WindowLength', 'LPTimeConstant']

def sane_to_bci2000(k, v):
	# Translation layer from the sane world (numbers are numbers and we know what they mean)
	# to the BCI2000 parameter world (numbers will be interpreted wrongly unless you remember to make them strings with the correct suffix)
	# For the 1003rd time:  grrrr.
	if k in [f+'Msec' for f in insane_fields]:
		if isinstance(v, list): v = [sane_to_bci2000(k,vi)[1] for vi in v]
		elif not str(v).lower().strip().endswith('ms'): v = str(v).strip()+'ms'
		k = k[:-4]
	return k,v

def bci2000_to_sane(k,v):
	if k in insane_fields:
		k += 'Msec'
		if isinstance(v, list): v = [bci2000_to_sane(k,vi)[1] for vi in v]
		else: v = float(str(v).rstrip('ms'))
	return k,v


class robotest(SigTools.experiment):
	def __init__(self, _baseobj=None, **kwargs):
		SigTools.experiment.__init__(self)
		
		params = """
			Controller                              list      ControllerExpressions=   1     Robot()       //
			Controller                              int       DiffController=                0             // (boolean)
			Controller                              int       SplitController=               1             // (boolean)
			Application:Game                        float     BallSpeedHitFactor=            1.00          // Multiplier for the ball speed when a hit occurs 
			Storage:Session:DataIOFilter            string    SubjectName=                   $NAME         // subject alias
			Storage:Session:DataIOFilter            string    SubjectSession=                $SESSION      // three-digit session number
			Storage:Session:DataIOFilter            string    SubjectRun=                    $RUN          // two-digit run number
			Application:Game                        matrix    Level=                 $ROWS $COLS     $MATRIX  // Define block placement on breakout level 
			
			Controller                              float     ControllerGain=                3.0           //
			Controller                              float     ControllerNoise=               0.0           //
			Controller                              float     MaxRobotOutput=                0.25          //
			Controller                              int       RobotIntelligence=             1             //
			
			Application:Game                        float     InitialGameDifficulty=       -10             // Initial width of the paddle rectangle (0-2) 
			Application:Game                        float     BallSpeed=                     4.0           // Initial ball speed (distance per second) 
			Filtering:ARFilter                      float     WindowLength=                300ms           // Time window for spectrum computation
			Filtering:LPFilter                      float     LPTimeConstant=                0ms           // time constant for the low pass filter
		"""
		nrows = 5
		ncols = 10
		matrix = numpy.ones((nrows,ncols)) * 2
		matrix[:, ::2] = 0.5
		matrix[::2,:] = numpy.c_[matrix[::2,1:],matrix[::2,0]]
		matrix = '     '.join([' '.join(['%g'%i for i in row]) for row in matrix])
		params = params.replace('$ROWS', str(nrows)).replace('$COLS', str(ncols)).replace('$MATRIX', matrix)
		
		params = [BCI2000Tools.FileReader.ParseParam(p) for p in params.split('\n') if len(p.strip())]
		params = SigTools.sstruct([ (p['name'],p) for p in params])
		self._params = params
		self._allowedfields = [bci2000_to_sane(k,0)[0] for k in params._allfields()]
		for k in [
			'ControllerGain', 'ControllerNoise', 'MaxRobotOutput', 'RobotIntelligence',
			'InitialGameDifficulty', 'BallSpeed', 'WindowLength', 'LPTimeConstant',
		]:
			v = params[k]['val']
			k,v = bci2000_to_sane(k,v)
			self._setfield(k, [v] )
		self.__dict__['_allowedfields'] = self._allfields() # narrow it further
		self._update(_baseobj)
		self._update(kwargs)
	
	def paramstring(self, x):
		import copy
		p = copy.deepcopy(self._params)
		for k,v in x._allitems():
			k,v = sane_to_bci2000(k, v)
			p[k]['name'] = k
			p[k]['val'] = v
		return '\n'.join(BCI2000Tools.FileReader.FormatPrmList([v for k,v in p._allitems()]))
	
	def write(self, directory=None):
		import md5,os
		strings = [self.paramstring(x) for x in self]
		datestamp = time.strftime('%Y%m%d_%H%M%S', time.localtime())
		hash = md5.md5('\n/////////////\n'.join(strings)).hexdigest().upper()
		name = '%s_%s_ROBOTEST_' % (datestamp, hash[:8])
		session = '001'
		if directory == None:
			directory = os.path.join(BreakoutDir, '..', '..', 'src', 'private', 'Application', 'Games', 'Breakout', 'breakout', name+session)
		directory = os.path.realpath(directory)
		if not os.path.exists(directory): os.mkdir(directory)
		
		wrapperfile = os.path.realpath(os.path.join(directory, name+session+'.bat'))
		wrapper = open(wrapperfile, 'w')		
		wrapper.write(r"""
@cd ..\..\
@set STARTDIR=%CD%
@cd ..\..\..\..\

cd ..\prog
""")
		print 'written',wrapperfile
		
		for i,content in enumerate(strings):
			run = '%03d' % (i+1)
			content = content.replace('$NAME', name).replace('$SESSION', session).replace('$RUN', run)
			filename = '%sS%sR%s.prm' % (name, session, run)
			filename = os.path.join(directory,filename)
			open(filename, 'w').write(content)
			print 'written',filename
			wrapper.write(r'call %STARTDIR%\breakout\batch\robotest.bat ' + filename + '\n')
			
	def collate(self, conditions):
		for field in self._allfields():
			self._setfield(field, [])
		for condition in conditions:
			self.__iadd__(condition[0])
		out = [None] * len(self)
		for condition in conditions:
			ind = [i for i,x in enumerate(self) if x == condition[0]]
			if len(ind) != 1: raise RuntimeError("huh?")
			out[ind[0]] = condition[1]
		return out
		
	def read(self, dirname='.'):
		files = BCI2000Tools.FileReader.ListDatFiles(dirname);
		conditions = []
		paramnames_b = [sane_to_bci2000(key_s,val_s)[0] for key_s,val_s in self._allitems()]
		for file in files:
			print file
			b = breakout(file)
			tt = b.Threshold()
			condition = []
			for key_b in paramnames_b:
				condition.append(bci2000_to_sane(key_b, b.params[key_b]))
			condition = SigTools.sstruct(condition)
			conditions.append([condition,tt])
		return self.collate(conditions)
				
				

class breakout(SigTools.sstruct):
	def __init__(self, filename=None, ind=-1):
	
		SigTools.sstruct.__init__(self)
		if filename == None: filename = BreakoutDir
		b = BCI2000Tools.FileReader.bcistream(filename, ind=ind)
		self.filename = b.filename
		self.datestamp = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(b.datestamp))
		signal, rawstates = b.decode('all')
		self.params = SigTools.sdict(b.params)
		self.rawstates = SigTools.sdict(rawstates)
		self.states = SigTools.sdict()
		dt,t = SigTools.unwrapdiff(self.rawstates.SourceTime.flatten(), 65536, startval=0, dtype=numpy.float64)
		for k,v in self.rawstates.items():
			v = numpy.asarray(v.T, numpy.float64)
			v = v[dt != 0, :]
			self.states[k] = v	
		self.states.SourceTime.flat[:] = t[dt != 0]


	def plot(self, states, *pargs, **kwargs):
		if not isinstance(states, (tuple,list)): states = [states]
		states = list(states)
		for i,v in enumerate(states):
			if isinstance(v, basestring): states[i] = self.states[v]
		states = numpy.concatenate(states, axis=1)
		return SigTools.plot(self.states.SourceTime, states, *pargs, **kwargs)
	
	def Threshold(self, x=None, alpha=0.05):
		if x == None: x = self.GameDifficulty()
		if isinstance(x, basestring): x = self.states[x]
		rev = self.states.Reversals
		points = 1 + numpy.where(numpy.diff(rev, axis=0))[0]
		x = x[points].flatten()
		rev = rev[points].flatten()
		hits   = [self.states.JustHit[self.states.Reversals == r].sum() for r in rev]
		misses = [self.states.JustMissed[self.states.Reversals == r].sum() for r in rev]
		
		nr = len(rev)
		nDiscarded = numpy.arange(0,nr)
		nUsed = numpy.arange(0,nr+1)
		z = numpy.zeros((len(nDiscarded), len(nUsed)), dtype=numpy.float64) + numpy.nan
		accuracy = z + 0.0
		lo = z + 0.0
		up = z + 0.0
		threshold = z + 0.0
		for id,discarded in enumerate(nDiscarded):
			for iu,used in enumerate(nUsed):
				if used == 0: continue
				if used % 2: continue
				if used + discarded > nr: continue
				threshold[id,iu] = numpy.median(x[discarded:discarded+used])
				nhits = float(sum(hits[discarded:discarded+used]))
				nmisses = float(sum(misses[discarded:discarded+used]))
				accuracy[id,iu] = nhits / (nhits + nmisses)
				lo[id,iu],up[id,iu] = ClopperPearson(nhits, nhits+nmisses, alpha)
				
		d = {
			'GameDifficulty': x,
			'ReversalIndex': rev,
			'Hits':   hits,
			'Misses': misses,
			'Threshold': threshold,
			'Accuracy': accuracy,
			'LowerLimit': lo,
			'UpperLimit': up,
		}
		return SigTools.sdict(d)
		
		
	def GameDifficulty(self, statename='PaddleWidth'):
		v0 = float(self.params[statename])
		factor = float(self.params[statename + 'HitFactor'])
		if statename in self.states:
			v = self.states[statename]
		else:
			v = self.states[statename + 'Times10000'] / 5000
		d = (numpy.log(v) - numpy.log(v0)) / numpy.log(factor)
		return d
