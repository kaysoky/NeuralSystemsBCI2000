# TODO: Segment object needs to be paired with info on which array(s) its data representation(s) live in, and which axis and slice it refers to in each array
# TODO: Segments might need to refer to other segments (two periods of the same trial...): maybe need new concept ("trial"/"exemplar")?  or the id of a segment needs to include its length...
# TODO: Dimension probably needs an array of ids as well as the optional vals...?

import os
import sys
import copy
import time
import numpy
import inspect

from SigTools import sstruct, ChannelSet

def datestamp2sec(val):
	if isinstance(val, numpy.ndarray): val = [x for x in val.flat]
	if isinstance(val, list): return [datestamp2sec(x) for x in val]
	if val == None: return None
	msec = 0
	if isinstance(val, (long,int,float)):
		msec = round(1000 * (val - int(val)))
		val = int(val)
		if msec == 1000: msec = 0; val += 1
		val = '%14d' % int(val)
		if msec: val += '.%03d' % msec
	err = ValueError("invalid timestamp '%s'" % str(val).strip())
	if isinstance(val, basestring):
		t = reduce(str.__add__, [x for x in val if x in '0123456789.'])
		try: tf = float(t)
		except: raise err
		if '.' in t: t,msec = t.split('.')
		else: msec = '0'
		msec = int(float('.' + msec)*1000)
		if msec != 0 and tf < 10000000000000: raise err  # Input must start at year at the left-hand end. If it goes down to msec, it must be YYYYmmDDHHMMSS 
		val = [0,1,1,0,0,0]
		if len(t) not in [4,6,8,12,14]: raise err
		if len(t): val[0] = int(t[:4]); t = t[4:]
		if len(t): val[1] = int(t[:2]); t = t[2:]
		if len(t): val[2] = int(t[:2]); t = t[2:]
		if len(t): val[3] = int(t[:2]); t = t[2:]
		if len(t): val[4] = int(t[:2]); t = t[2:]
		if len(t): val[5] = int(t[:2]); t = t[2:]
		val = tuple(val)
	if not isinstance(val, tuple): raise err
	if len(val) not in [1,2,3,5,6,9]: raise err
	val = list(val)
	val += [1] * (3-len(val))  # month 1 and/or day 1 if missing
	val += [0] * (9-len(val))  # hour 0, minute 0 and/or second 0 if missing
	if val[5] > int(val[5]): msec = round(1000.0 * (val[5]-int(val[5]))); val[5] = int(val[5])
	if not 1 <= val[1] <= 12: raise err
	if not 1 <= val[2] <= 31: raise err
	if not 0 <= val[3] <= 23: raise err
	if not 0 <= val[4] <= 59: raise err
	if not 0 <= val[5] <= 61: raise err
	return time.mktime(val)	+ msec/1000.0

def sec2datestamp(val):
	if isinstance(val, (numpy.ndarray)): val = [x for x in val.flat]
	if isinstance(val, (tuple,list)): return [sec2datestamp(x) for x in val]
	if val == None: return None
	lt = list(time.localtime(val))
	t = time.mktime(lt)
	msec = int(round(1000 * (val-t)))
	if msec == 1000: lt[5] += 1; msec = 0
	s = '%04d-%02d-%02d %02d:%02d:%02d' % tuple(lt[:6])
	if msec: s += '.%03d' % msec
	return s

def hms2sec(hmsstring):
	t = hmsstring.split(':')
	if len(t) > 3: raise ValueError, 'too many colons in timestamp "%s"' % hmsstring
	t.reverse()
	t = [float(x) for x in t] + [0]*(3-len(t))
	t = t[0] + 60.0 * t[1] + 3600.0 * t[2]
	return int(round(t * self.samplingfreq_hz))
	
def sec2hms(sec, positiveSignString=''):
	msec = round(1000.0 * float(sec))
	if msec < 0.0: signstr = '-'; msec = -msec
	else: signstr = positiveSignString
	sec,msec = divmod(int(msec), 1000)
	mins,sec  = divmod(int(sec), 60)
	hours,mins  = divmod(int(mins), 60)
	return '%s%02d:%02d:%02d.%03d' % (signstr,hours,mins,sec,msec)

class Timespan(object):
	def __init__(self, val=None, units=None, **kwargs):
		"""
		Timespan(3, 'seconds')		
		Timespan(samples=100, fs=256, unit='seconds')
		Timespan(seconds=0.9, fs=256)
		Timespan(hr=2, min=30, sec=36)
		
		"""###
		self.__samples = 0.0
		self.__units = None
		self.__fs = 1000.0
		self.__blocksize = 1000.0
		for k,v in kwargs.items():
			if self.__legalunits(k): setattr(self, k, getattr(self, k) + v)
			else:
				prop = getattr(self.__class__, k, None)
				if prop is self.__class__.units: units = v # defer this, just in case fs is set while units has been changed to 'samples': that would transform the length
				elif isinstance(prop, property): setattr(self, k, v)
				else: raise ValueError('units "%s" not recognized' % str(k))
		self.units = units
		if val != None: self.set(self.get()+val)

	@classmethod
	def __legalunits(cls, units):
		if units == None: return True
		return isinstance(units, basestring) and isinstance(getattr(cls, units, None), cls.__unitproperty)
	def _insamples(self, units=()):
		if units == (): units = self.__units
		return units != None and getattr(self.__class__, units, None) is self.__class__.samples
	def _inpackets(self, units=()):
		if units == (): units = self.__units
		return units != None and getattr(self.__class__, units, None) is self.__class__.packets

	def __repr__(self):
		return '<%s object at 0x%08X>: %s' % (self.__class__.__name__, id(self), str(self))	

	def __str__(self):
		return self.report()
		
	def report(self, units=(), positiveSignString=''):
		if units == (): units = self.__units
		if units != None and hasattr(self, units):
			v = getattr(self, units)
			if v == 1 and len(units) > 1 and units.endswith('s'): units = units[:-1]
			if v < 0: positiveSignString = ''
			s = '%s%g %s' % (positiveSignString, v, units)
			if self._insamples(units): s += ' @ %g Hz' % self.fs
			if self._inpackets(units):
				r = self.__rounding;
				if r == 0.0: r = 1.0
				s += ' of %s @ %g Hz' % (plural(r, 'sample', True), self.fs) 
			return s
		return sec2hms(self.sec)
				
	def __iadd__(self, other):
		if isinstance(other, Timespan):
			self.seconds += other.seconds
		else:
			units = self.__units
			if units == None: units = 'seconds'
			setattr(self, units, getattr(self, units) + other)
		return self
	def __isub__(self, other): return self.__iadd__(-other)
	def __imul__(self, val): self.samples *= val; return self
	def __idiv__(self, val): self.samples /= val; return self
	
	def __mul__(self, other): x = copy.deepcopy(self); x *= other; return x
	def __rmul__(self, other): x = copy.deepcopy(self); x *= other; return x
	def __div__(self, other): x = copy.deepcopy(self); x /= other; return x
	def __add__(self, other): x = copy.deepcopy(self); x += other; return x
	def __sub__(self, other): x = copy.deepcopy(self); x -= other; return x
	def __rsub__(self, other): x = -1.0 * self; x += other; return x
	def __neg__(self): return -1.0 * self
	def __pos__(self): return +1.0 * self
	
	def __cmp__(self, other): return cmp(self.sec, other.sec)
	
	def __float__(self):
		return self.get()
	
	def get(self, units=None):
		if units == None: units = self.__units
		if units == None: units = 'sec'
		elif not self.__legalunits(units): raise ValueError('unrecognized units "%s"' % str(units))
		return getattr(self, units)

	def set(self, val, units=None):
		if isinstance(val, Timespan): self.seconds = val.seconds; return
		if units == None: units = self.__units
		if units == None: units = 'seconds'
		elif not self.__legalunits(units): raise ValueError('unrecognized units "%s"' % str(units))
		setattr(self, units, val)
	
	def resample(self, fs):
		self.samples *= fs / self.__fs
		self.__fs = float(fs)

	doc = """time units in which to perform arithmetic and format output"""###
	def fget(self): return self.__units
	def fset(self, val):
		if not self.__legalunits(val): raise ValueError('unrecognized units "%s"' % str(val))
		self.__units = val
	unit = units = property(fget, fset, doc=doc)

	doc = """number of samples in one packet or sample-block"""###
	def fget(self): return self.__blocksize
	def fset(self, val):
		val = float(val)
		if val <= 0.0: raise ValueError("packet size must be > 0")
		if self._inpackets(): self.samples = round(self.samples * val / self.__blocksize)
		self.__blocksize = val
	SampleBlockSize = block_size = packet_size = property(fget, fset, doc=doc)
	
	doc = """sampling frequency"""###
	def fget(self): return self.__fs
	def fset(self, val):
		val = float(val)
		if val <= 0.0: raise ValueError("sampling frequency must be > 0")
		if self._insamples() or self._inpackets(): self.__fs = float(val)
		else: self.resample(val)
	SamplingRate = samplingfreq_hz = fs = property(fget, fset, doc=doc)
	
	class __unitproperty(property): pass
	doc = """time in weeks"""### well, why not?
	def fget(self): return self.seconds / 604800.0
	def fset(self, val): self.seconds = val * 604800.0
	wk = wks = weeks = __unitproperty(fget, fset, doc=doc)
	doc = """time in days"""###
	def fget(self): return self.seconds / 86400.0
	def fset(self, val): self.seconds = val * 86400.0
	days = __unitproperty(fget, fset, doc=doc)
	doc = """time in hours"""###
	def fget(self): return self.seconds / 3600.0
	def fset(self, val): self.seconds = val * 3600.0
	hr = hrs = hours = __unitproperty(fget, fset, doc=doc)
	doc = """time in minutes"""###
	def fget(self): return self.seconds / 60.0
	def fset(self, val): self.seconds = val * 60.0
	min = mins = minutes = __unitproperty(fget, fset, doc=doc)
	doc = """time in seconds"""###
	def fget(self): return self.samples / self.__fs
	def fset(self, val): self.samples = val * self.__fs
	s = sec = seconds = __unitproperty(fget, fset, doc=doc)	
	doc = """time in milliseconds"""###
	def fget(self): return self.seconds * 1000.0
	def fset(self, val): self.seconds = val / 1000.0
	msec = milliseconds = __unitproperty(fget, fset, doc=doc)
	doc = """time in samples"""###
	def fget(self): return self.__samples
	def fset(self, val): self.__samples = float(val)
	samples = __unitproperty(fget, fset, doc=doc)
	doc = """time in packets"""###
	def fget(self): return self.samples / self.__blocksize 
	def fset(self, val): self.samples = val * self.__blocksize
	SampleBlocks = blocks = packets = __unitproperty(fget, fset, doc=doc)
	
	
class Segment(object):

	def __init__(self, **kwargs):
		self.__offset = Timespan()
		self.__length = Timespan()
		self.source = None
		self.t0 = None
		self.parent = None
		self.type = None
		self.number = None
		self.__specify(**kwargs)

	def __specify(self, **kwargs):
		oval, ounits = None, None
		lval, lunits = None, None
		for k,v in kwargs.items():
			if isinstance(getattr(Segment, k, None), property):
				setattr(self, k, v)
				continue
			elif k in self.__dict__ and not k.startswith('_'):
				setattr(self, k, v)
				continue
			elif '_' in k:
				addr,units = k.split('_', 1)
				if len(units) and addr in ['offset']:
					if oval != None: raise ValueError('multiple offset_* arguments')
					oval,ounits = v, units
					continue
				if len(units) and addr in ['length', 'duration']:
					if lval != None: raise ValueError('multiple length_* or duration_* arguments')
					lval,lunits = v, units
					continue
			raise ValueError('argument "%s" not recognized' % k)
		if oval != None: self.__offset.set(oval, units=ounits) # set these last, once fs and packet_size and units are decided
		if lval != None: self.__length.set(lval, units=lunits)

	def __contains__(self, other):
		if isinstance(other, Segment):
			return self.source == other.source and other.start >= self.start and other.end <= self.end # yes, end less than or equal
		else:
			return other >= self.start and other < self.end # yes, end strictly less than
		
	doc="""offset relative to t0"""###
	def fget(self): return self.__offset
	def fset(self,val): self.__offset.set(val)
	offset = property(fget,fset,doc)

	doc="""length or duration of the segment"""###
	def fget(self): return self.__length
	def fset(self,val): self.__length.set(val)
	length = duration = property(fget,fset,doc=doc)

	doc="""
t0 is a datestamp, in seconds since the epoch a la time.time(), from
which offsets are measured. This property may also be set using a tuple
of the form returned by time.localtime(). """###
	def fget(self): return self.__t0
	def fset(self,val):
		if not isinstance(val, float): val = datestamp2sec(val)
		self.__t0 = val
	t0 = property(fget,fset,doc=doc)
	
	doc="""(source string, start datestamp, duration in seconds)"""###
	def fget(self):
		fac = 10.0 ** numpy.floor(numpy.log10(self.fs))
		return self.source, round(self.start*fac)/fac, round(self.length.seconds*fac)/fac
	def fset(self,val): raise RuntimeError("id is a read-only property")
	id = property(fget,fset,doc=doc)
	
	doc="""start is a read-only property: t0 + offset"""###
	def fget(self):
		t0 = self.t0
		if t0 == None: t0 = 0.0
		return t0 + self.offset.sec
	def fset(self,val): raise RuntimeError("start is a read-only property")
	start = property(fget,fset,doc=doc)
	
	doc="""end is a read-only property: t0 + offset + length"""###
	def fget(self):
		t0 = self.t0
		if t0 == None: t0 = 0.0
		return t0 + self.offset.sec + self.length.sec
	def fset(self,val): raise RuntimeError("end is a read-only property")
	end = property(fget,fset,doc=doc)
	
	doc="""sampling rate in Hz"""###
	def fget(self):
		val = self.__offset.fs
		if self.__length.fs != val: raise RuntimeError("length and offset have different sampling frequencies")
		return val
	def fset(self,val): self.__offset.fs = self.__length.fs = val
	SamplingRate = samplingfreq_hz = fs = property(fget,fset,doc=doc)
	
	doc="""units"""###
	def fget(self):
		val = self.__offset.units
		if val == None: val = self.__length.units
		elif self.__length.units != val: raise RuntimeError("length and offset have different units")
		return val
	def fset(self,val):
		self.__offset.units = self.__length.units = val
		if not self.__offset._insamples() and not self.__offset._inpackets(): self.__offset.units = None
	unit = units = property(fget,fset,doc=doc)
	
	
	def __iadd__(self, val): self.offset += val; return self
	def __isub__(self, val): self.offset -= val; return self
	
	def __repr__(self):
		return '<%s object at 0x%08X>: %s' % (self.__class__.__name__, id(self), str(self))	

	def __str__(self):
		if self.offset._insamples() or self.offset._inpackets(): u = None
		else: u = 'hms'
		s = 'offset %s, duration %s' % (self.offset.report(units=u, positiveSignString='+'), self.length)
		if self.t0 != None: s += ' (relative to %s)' % sec2datestamp(self.t0)
		if self.source != None: s += ' -- source: %s' % str(self.source)
		return s
	
	def rebase(self, t0):
		"""
		Change t0, but keep the same absolute start and end time by adjusting the offset.
		"""###
		t0 = getattr(t0, 'start', t0)
		oldt0 = self.t0
		self.t0 = t0
		shift = oldt0 - self.t0
		self.offset.seconds += shift
		
	def spawn(self, **kwargs):
		s = copy.deepcopy(self)
		s.parent = self.id
		got_t0 = self.t0 not in [0, None]
		if got_t0: kwargs['t0'] = kwargs.get('t0', self.start)
		else: s.offset = 0
		s.__specify(**kwargs)
		if not got_t0: s.offset += self.offset		
		return s

class SegmentSet(numpy.matrix):
	def __new__(cls, s=None):
		if s == None: self = []
		else: self = copy.deepcopy(s)
		if isinstance(s, numpy.ndarray) and len(s.shape) == 2: shape = s.shape
		else:
			if isinstance(s, numpy.ndarray): s = s.flatten()
			if s == None: shape = (0,1)
			else: shape = (len(s),1)
		self = numpy.matrix(self, dtype=Segment).view(cls)
		self.shape = shape
		return self

class Dimension(object):
	def __init__(self, slicename, dimname=None, units=None, vals=None):
		self.slicename = copy.deepcopy(slicename)
		self.dimname = copy.deepcopy(dimname)
		self.units = copy.deepcopy(units)
		vals = copy.deepcopy(vals)
		if not isinstance(vals, (numpy.ndarray,type(None))):
			vals = numpy.asarray(vals, dtype=numpy.float64)
		self.vals = vals

	def __repr__(self):
		return '<%s object at 0x%08X>: %s' % (self.__class__.__name__, id(self), str(self))	
	def __str__(self):
		s = '%s dimension' % self.slicename
		if self.dimname is None and self.units is None and self.vals is None: return s
		s += ' ('
		if self.dimname is not None: s += '%s ' % self.dimname
		if self.units is not None: s += 'in %s ' % self.units
		if self.vals is not None:
			try: s += 'from %g to %g' % (v.flat[0],v.flat[-1])
			except: pass
		return s.strip() + ')'

class Preprocessing(object):
	def __init__(self, method, summary='??', opts=None, info=None, testmethod=None, testopts=None, datestamp=None, **kwargs):
		self.method = method
		self.testmethod = testmethod
		if opts == None: opts = {}
		opts.update(kwargs)
		self.opts = DotDict(copy.deepcopy(opts))
		if testopts == None: testopts = {}
		self.testopts = DotDict(copy.deepcopy(testopts))
		self.summary = summary
		if info == None: info = {}
		self.info = sstruct(copy.deepcopy(info))
		self.channels = ChannelSet()
		self.segments = SegmentSet()
		self.dims = listof(type=Dimension)
		if datestamp == None: datestamp = time.localtime()
		if not isinstance(datestamp, float): datestamp = time.mktime(datestamp)
		self.datestamp = datestamp

	def __repr__(self):
		return '<%s object at 0x%08X>: %s' % (self.__class__.__name__, id(self), str(self))	
	def __str__(self):
		return self.summary
	
	def dimstr(self):
		return 'TODO'
	
class bci(object):
	def __init__(self, d=None):
		self.__dict__['prep'] = listof(type=Preprocessing)
		self.__dict__['fulldir'],self.__dict__['shortdir'] = FindDir(d)	
	
	def file(self, *pargs):
		return os.path.join(self.fulldir, *pargs)
		
	def rawformat(self, format=None, prefix='raw_'):
		if format == None:
			d = os.listdir(self.fulldir)
			d = sorted([x for x in d if os.path.isdir(os.path.join(self.fulldir,x)) and x.startswith(prefix)])
			if len(d) == 0: raise IOError("found no %s* directories under '%s'" % (prefix,self.fulldir))
			if len(d) > 1: print "WARNING: found multiple raw formats for %s: taking %s" % (self.shortdir,os.path.split(d[0])[-1])
			format = d[0]
		if format.startswith(prefix): format = format[len(prefix):]
		return format
		
	def rawdir(self, format=None, prefix='raw_'):
		if format == None: format = self.rawformat(prefix=prefix)
		d = self.file(format)
		if not os.path.isdir(d) and not format.startswith(prefix): format = prefix + format; d = self.file(format)
		if not os.path.isdir(d): raise IOError("failed to find directory '%s'" % d)
		return os.path.realpath(d)
	
	def rawfiles(self, format=None, include=None, exclude=None):
		d = self.rawdir(format=format)
		f = os.listdir(d)
		if include != None:
			if isinstance(include, basestring): include = [include]
			f = [x for x in f if x.lower().endswith(tuple(include))]
		if exclude != None:
			if isinstance(exclude, basestring): exclude = [exclude]
			f = [x for x in f if not x.lower().endswith(tuple(exclude))]
		f = [os.path.join(d,x) for x in f]
		f = [x for x in f if os.path.isfile(x)]
		if len(f): f = zip(*sorted(zip(SessionRun(f),f)))[-1]
		return f
	
	def template(self, opt1='default1', opt2='default2', **passthru):
		prep,opts,info = self.makeprepinfo()
		prep.summary = 'whatever'
		info.foo.bar = 1
		return self.addprepinfo(prep)
		
	def readraw(self, format=None, include=None, exclude=None, select=None, maxfiles=None, **passthru):
		prep,opts,info = self.makeprepinfo()
		opts.format = format = self.rawformat(format=format)
		global RAW_READERS
		reader = RAW_READERS.get('readraw_' + opts.format)
		if reader == None: raise ValueError("no readraw_%s function found" % opts.format)
		
		files = self.rawfiles(format=opts.format, include=opts.include, exclude=opts.exclude)
		if len(files) == 0: sessions,runs = [],[]
		else: sessions,runs = zip(*SessionRun(files))
		files = zip(sessions,runs,files)
		if select != None:
			if isinstance(select, (tuple,list)):
				if len(set(sessions)) == 1: select = {sessions[0]:select}
				else: raise ValueError('more than one session detected %s: <select> should be a dict with session numbers as keys' % str(tuple(set(sessions))))
			selected = []
			for session,run,file in files:
				allow = select.get(session,[])
				if allow == 'all' or run in allow: selected.append((session,run,file))
			files = selected
		if len(files) == 0: raise RuntimeError("no matching files found in raw_%s directory" % opts.format)

		used = []; xx = []
		if maxfiles != None: files = files[:maxfiles]
		ss = {}; rr = []
		for session,run,file in files:
			x = reader(file, **passthru)
			if x == None: continue
			used.append((session,run,file))
			x = sstruct(x)
			xx.append(x)
			r = x._getfield('run')
			r.units = None; r.session = session; r.number = run
			s = ss.get(session)
			if s == None: s = ss[session] = r.spawn(type='session', number=session, parent=None)
			if s.number == None: s.number = x._getfield('session_number', None)
			s.t0 = min(s.t0, r.t0); s.length.sec = max(s.length.sec, r.end - s.start)
			r.rebase(s.start); r.parent = s.id; rr.append(r)
			r.file = file
			# TODO:  check the compatibility of fs, channels across different x's 
			
			
		if len(used) == 0: raise RuntimeError("no data!")		
		prep.info.files = zip(*used)[-1]
		t = prep.info.TODO_REMOVE_THIS
		t.segments.sessions = sorted(ss.values())
		t.segments.runs = sorted(rr)
		
		summ = {}
		for session,run,file in used:
			s = summ[session] = summ.get(session,[])
			if not run in s: s.append(run)
		prep.summary = 'read raw files ['
		prep.summary += '; '.join(['S%s:%s' % ({None:'?'}.get(session,session),intseqstr(sorted(runs))) for session,runs in sorted(summ.items())])
		prep.summary += ']'
		
		summary = x._getfield('summary', '')
		if summary: prep.summary += ' ' + summary 
		prep.opts.update(x.opts)
		prep.info._update(x._getfield('info',{}))
		return self.addprepinfo(prep)
	
	def addprepinfo(self, prep):
		self.prep.append(prep)
		return self
		
	def makeprepinfo(self, summary='', method=None, opts=None, info=None, testmethod=None, testopts=None, datestamp=None, caller_depth=0, **kwargs):
		if method == None or opts == None:
			cmethod,cargs,cpargs,ckwargs = caller(check_self=self, include_kwargs=True, depth=caller_depth)
			for k,v in cargs.items():
				if v is self: cargs.pop(k)
		if opts == None: opts = cargs
		if method == None: method = cmethod
		else:
			if isinstance(method, basestring):
				method = {'module':None, 'func':method, 'class':None}
			else:
				cl = getattr(method, 'im_class', None)
				mod = getattr(method, '__module__', getattr(cl, '__module__',None))
				method = {'module':getattr(mod,'__name__',None),'func':getattr(method,'__name__',None), 'class':getattr(cl,'__name__',None)}
		method.pop('obj',None)
		method = DotDict(method)
		if testmethod != None: testmethod = DotDict(testmethod)
		if summary == '': summary = method.get('func','')
		
		p = Preprocessing(summary=summary, method=method, opts=opts, info=info, testmethod=testmethod, testopts=testopts, **kwargs)
		if len(self.prep):
			prev = self.prep[-1]
			p.channels = copy.deepcopy(prev.channels)
			p.segments = copy.deepcopy(prev.segments)
			p.dims = copy.deepcopy(prev.dims)
		return p,p.opts,p.info
		
	def __getattr__(self, key):
		#if key in self.__dict__: return self.__dict__[key]
		pp = self.__dict__.get('prep')
		if pp != None and len(pp) > 0:
			p = pp[-1]
			if hasattr(p, key): return getattr(p, key)
		raise AttributeError(key)

	def __setattr__(self, key, val):
		if hasattr(self, key):
			self.__dict__[key] = val
			return
		if len(self.prep):
			p = self.prep[-1]
			if hasattr(p, key): return setattr(p, key, val)
		raise AttributeError(key)

	def _getAttributeNames(self):
		if len(self.prep): return [x for x in self.prep[-1].__dict__ if not x.startswith('_')]
		else: return []

	def __repr__(self):
		ref = '<%s object at 0x%08X>' % (self.__class__.__name__, id(self))
		s = str(self)
		if len(s): return '\n    '.join([ref] + str(self).split('\n'))
		else: return ref
		
	def __str__(self):
		s = [[
			0, None, 'c', time.strftime('%a %Y-%m-%d %H:%M:%S', time.localtime(p.datestamp)),
			2, None, 'c', p.dimstr(),
			2, None, 'r', ('%d:' % i),
			1, 40,   'l', str(p), # TODO: wordwrap
		] for i,p in enumerate(self.prep)]
		if len(s): s = justify(s)
		else: s = ''
		if self.shortdir != None: s = '%s\n%s' % (self.shortdir,s)
		return s

	def foo(self, summarystring='method foo'): # TODO temporary
		prep,opts,info = makeprepinfo(summary=summarystring)
		return self.addprepinfo(prep)

def foo(self, summarystring='function foo'): # TODO temporary
	prep,opts,info = makeprepinfo(summary=summarystring)
	return self.addprepinfo(prep)




def myargs(include_kwargs=False, include_args_that_have_no_defaults=False):
	info,args,pargs,kwargs = caller(include_kwargs=include_kwargs)
	func = info['obj']
	argnames,pargname,kwargname,defaults = inspect.getargspec(func)
	if not include_args_that_have_no_defaults:
		dict([(k,args[k]) for k in argnames[-len(defaults):]])
	return args
	
def caller(check_self=None, include_kwargs=True, depth=0):
	caller_frame = inspect.stack(1)[depth+2][0]
	[argnames, pargname, kwargname, workspace] = inspect.getargvalues(caller_frame)
	pargs = workspace.get(pargname, ())
	kwargs = workspace.get(kwargname,{})
	args = dict([(a,workspace[a]) for a in argnames if a in workspace and a not in ['self', pargname, kwargname]])
	if include_kwargs: args.update(kwargs)
	caller_name = caller_frame.f_code.co_name
	module_name = caller_frame.f_globals.get('__name__')
	caller_self = workspace.get('self') # assumes self is called 'self', which it should be but doesn't strictly have to be
	obj = methodobj = funcobj = None
	firstline = caller_frame.f_code.co_firstlineno # an indirect strategy like this seems to be the only way to disambiguate between module.func and module.class.method if both have the same name and an argument 'self' 
	funcobj = getattr(sys.modules[module_name], caller_name, None)
	if caller_self == None:
		cl = class_name = None
	elif check_self is not None and caller_self is not check_self:
		raise RuntimeError('who?')
	else:
		cl = caller_self.__class__
		class_name = getattr(cl, '__name__', None)
		methodobj = getattr(cl, caller_name, None)
	if methodobj != None and methodobj.im_func.func_code.co_firstlineno == firstline: obj = methodobj
	elif funcobj != None and funcobj.func_code.co_firstlineno == firstline: obj = funcobj; class_name = None
	return {'module':module_name, 'func':caller_name, 'class':class_name, 'obj':obj},args,pargs,kwargs
	
	
def justify(x):
	spc       = x[0][0::4]
	wrapwidth = x[0][1::4]
	mode      = x[0][2::4]
	x         =  [xi[3::4] for xi in x]
	
	mode = [{'L':1.0,  'C':0.5, 'R':0.0,}[m.upper()] for m in mode]
	maxlen = [0 for xij in x[0]]
	nlines = [1 for xi in x]
	for i, xi in enumerate(x):
		for j, xij in enumerate(xi):
			if wrapwidth[j] != None: xij = wrap(xij, wrapwidth[j])
			xij = xij.split('\n')
			nlines[i] = max(nlines[i], len(xij))
			maxlen[j] = max(maxlen[j], max(map(len,xij)))
			x[i][j] = xij
	expanded = []
	for i, xi in enumerate(x):
		for k in range(nlines[i]):
			row = []
			for j, xij in enumerate(xi):
				if k < len(xij):
					n = maxlen[j] - len(xij[k])
					rij = int(round(mode[j] * n))
					lij = n - rij + spc[j]
					xijk = ' ' * lij + xij[k] + ' ' * rij
				else:
					xijk = ' ' * (maxlen[j]+spc[j])
				row.append(xijk)
			expanded.append(row)
	return '\n'.join([''.join(xi) for xi in expanded])

def wrap(text, width):
	"""
	A word-wrap function that preserves existing line breaks
	and most spaces in the text.

	Similar, although slightly superior, to textwrap.wrap with
	replace_whitespace=False and break_long_words=False
	
	code.activestate.com Recipe 148061 by Mike Brown
	"""###
 	def wraphelper(line, word, width=width):
		return	'%s%s%s' % (line, ' \n'[(len(line)-line.rfind('\n')-1 + len(word.split('\n',1)[0]) >= width)], word)
	text = text.replace('\r\n', '\n').replace('\r', '\n')
	return reduce(wraphelper, text.split(' '))

def plural(number, noun, include_number='%g', override=False):
	s = noun
	if number != 1.0 and not override:
		noun = noun.lower()
		if 0: pass
		elif noun.endswith(('sis')): s = s[:-2]; ending = 'es'
		elif noun.endswith(('x','ss')) and len(noun) > 1: ending = 'es'
		elif noun.endswith(('s')) and len(noun) > 1: ending = 'ses'
		elif noun.endswith(('y')) and len(noun) > 2 and not noun[:-1].endswith(('a','e','i','o','u')): s = s[:-1]; ending = 'ies'
		elif noun.endswith(('um')) and len(noun) >= 5: s = s[:-2]; ending = 'a'
		else: ending = 's'
		if len(s) and s[-1] != s[-1].lower(): ending = ending.upper()
		s += ending
		
	if include_number in [True]: include_number = '%g'
	if include_number not in [False, None, '']: s = (include_number % number) + ' ' + s
	return s

def intseqstr(vals, sort=True):
	if sort: vals = sorted(vals)
	if len(vals) == 0: return ''
	inner = []; outer = [inner]
	for i in vals:
		if len(inner) > 0 and i != inner[-1] + 1: inner = []; outer.append(inner)
		inner.append(i)	
	def joinem(inner):
		inner = [{None:'?'}.get(x,str(x)) for x in inner]
		if   len(inner) == 1: return '%s' % (inner[0])
		elif len(inner) == 2: return '%s,%s' % (inner[0], inner[-1])
		else:                 return '%s-%s' % (inner[0], inner[-1])
	return ','.join([joinem(inner) for inner in outer])

def SessionRun(f):
	if isinstance(f, (tuple,list)): return [SessionRun(x) for x in f]
	f = os.path.splitext(os.path.split(f)[-1])[0].lower()
	s,b = None,None
	import re
	match = re.compile('.*s([0-9]+).*').match(f)
	if match != None: s = int(match.groups()[0])
	match = re.compile('.*[0-9][rb_]+([0-9]+).*').match(f)
	if match != None: b = int(match.groups()[0])
	if s == None and b == None:
		match = re.compile('.*([0-9]+)$').match(f)
		if match != None: s,b = 1, int(match.groups()[0])
	return s,b
	
SESSION_DATAROOT = None
def DataRoot(*pargs):
	global SESSION_DATAROOT
	if len(pargs) > 1: raise TypeError("too many input arguments")
	if len(pargs) == 1:
		d = pargs[0]
		if d != None: d = os.path.abspath(d)
		SESSION_DATAROOT = d
	d = SESSION_DATAROOT
	if d == None:
		d = os.path.join(os.environ['HOME'], 'bci', 'data')
		if os.path.isdir(d): return d
		d = os.path.join(os.environ['HOME'], 'work', 'bci', 'data', 'dsroot')
		if os.path.isdir(d): return d
		raise RuntimeError('could not find data root directory')
	return d
	
def MatchShortcut(d, root=None, shortcutfile='shortcuts', reverse=False):
	if not os.path.isabs(shortcutfile):
		if root == None: root = DataRoot()
		stry = os.path.join(root, 'shortcuts')
		if os.path.isfile(stry): shortcutfile = stry
	lookup = {}
	reverse_lookup = {}
	try: fh = open(shortcutfile)
	except IOError: return d
	for line in fh.readlines():
		line = line.split('#',1)[0].strip()
		if len(line) == 0: continue
		shortcut,target = line.split('->',1)
		shortcut = shortcut.strip()
		try: n = int(shortcut)
		except:
			try: n = float(shortcut)
			except: n = None
		target = target.strip().replace('\\', '/').replace('/', os.path.sep)
		lookup[shortcut] = target
		if n != None: lookup[n] = target
		r = reverse_lookup[target] = reverse_lookup.get(target, [])
		if n == None or shortcut != str(n): r.append(shortcut)
		else: r.append(n)
	if reverse:
		fulld,shortd = FindDir(d, use_shortcuts=False)
		return reverse_lookup.get(shortd,[])
	else:
		return lookup.get(d,d)
	
def FindDir(d='', use_shortcuts=True):
	if d == None: return None,None
	d = str(d)
	root = DataRoot()
	if use_shortcuts: d = MatchShortcut(d, root=root)
	d = d.replace('\\', '/').replace('/', os.path.sep)
	if d.startswith('~'): import posixpath; d = posixpath.expanduser(d)

	if not os.path.isabs(d) and not d.startswith('.'):
		dtry = os.path.realpath(os.path.join(root,d))
		if os.path.isdir(dtry): d = dtry
	if not os.path.isabs(d):
		dtry = os.path.abspath(d)
		if os.path.isdir(dtry): d = dtry

	if not os.path.exists(d):
		dtmp = d; partial = []; prevlen = 0
		while len(dtmp) and len(dtmp) != prevlen: prevlen = len(dtmp); [dtmp,di] = os.path.split(dtmp); partial.append(di)
		if len(dtmp): partial.append(di)
		partial.reverse()
		for i in range(len(partial)):
			dtry = os.path.join(root, *partial[i:])
			if os.path.exists(dtry):
				print 'WARNING: can only match %s by replacing old root %s with new root %s' % (os.path.join(*partial[i:]), os.path.join(*partial[:i]), root)
				d = dtry
				break
	
	if not os.path.isdir(d):
		if os.path.exists(d): raise OSError('found %s but it is not a directory' % d)
		elif os.path.isabs(d): raise OSError('failed to find directory %s' % d)
		else: raise OSError('failed to find directory "%s" under root "%s"' % (d, root))
	d = os.path.realpath(d)
	root = os.path.realpath(root)
	
	
	if d == root: rootstripped = '.'
	else:
		root = os.path.join(root, '')
		if d.startswith(root): rootstripped = d[len(root):]
		else: rootstripped = d
	return d, rootstripped

RAW_READERS = {}
def RawReader(func):
	global RAW_READERS
	RAW_READERS[func.__name__] = func
	return func

@RawReader
def readraw_jez2000(file, trigger=None, trigger_vals=None, offset_msec=0, duration_msec=None, label_state=None):
	# opts, info, summary,    session_number, run, samplingfreq_hz, channels, datestamp,     data, segments, params
	if not file.lower().endswith('.dat'): return None
	if trigger==None: raise ValueError("trigger state or channel must be specified")	
	s = sstruct()
	
	try: from BCI2000Tools.FileReader import bcistream
	except ImportError: from BCPy2000.BCI2000Tools.FileReader import bcistream
	
	print 'reading %s' % file
	b = bcistream(file)
	fs = s.samplingfreq_hz = b.samplingfreq_hz
	source = b.params.get('SubjectName')
	if source != None:
		source = source.upper()
		while len(source) and source[-1] in '-_': source = source[:-1]
		while len(source) and source[0] in '0123456789-_': source = source[1:]
	s.run = Segment(type='run', t0=b.datestamp, source=source, fs=fs)
	s.run.length.samples = b.samples()
	try: s.run.number = int(b.params['SubjectRun'])
	except: pass
	try: s.session_number = int(b.params['SubjectSession'])
	except: pass

	chn = b.params.get('ChannelNames', [])
	ref = b.params.get('ReferenceChannelName', [])
	if not isinstance(ref, (tuple,list)): ref = [ref]
	if len(ref) == 0: ref = [None]
	if len(ref) == 1: ref = ref * len(chn)
	gnd = b.params.get('GroundChannelName', [])
	if not isinstance(gnd, (tuple,list)): gnd = [gnd]
	if len(gnd) == 0: gnd = [None]
	if len(gnd) == 1: gnd = gnd * len(chn)
	s.channels = ChannelSet(zip(chn,ref,gnd))
	
	state_trigger = triggerstr = channel_trigger = None
	if trigger in b.statedefs:
		state_trigger = triggerstr = trigger
	else:
		lab = s.channels.get_labels(lower=True)
		if not isinstance(trigger, (tuple,list)): trigger = [trigger]
		triggerweights = numpy.asmatrix(numpy.zeros((1,b.nchan), dtype=numpy.float64))
		triggerstr = []
		for t in trigger:
			if isinstance(t, str):
				try: ind = lab.index(t.lower())
				except: raise ValueError("could not match trigger state or channel '%s'" % t)
				triggerstr.append(t)
			else:
				ind = t - 0 # watch out: 0-based (but anyway, don't do this)
				if len(lab): triggerstr.append(lab[ind])
				else: triggerstr.append('ch#%03d' % (ind + 0)) # watch out: 0-based (but anyway, don't do this)
			triggerweights.flat[ind] = 1.0
		triggerstr = '+'.join(triggerstr)
	
	if not isinstance(trigger_vals, (tuple,list,type(None))): trigger_vals = [trigger_vals]

	if duration_msec == None: raise ValueError("duration_msec must be specified")
	duration_samples = b.msec2samples(duration_msec)
	duration_msec = b.samples2msec(duration_samples)
	
	[signal, states] = b.decode('all')

	if state_trigger == None:
		if trigger_vals == None: trigger_vals = [0]
		trsig = (triggerweights * signal).flat
		trsig = (trsig >= min(trigger_vals))
		trsig = numpy.logical_and(trsig, sigdiff(trsig)) # above threshold, and whether-above-threshold has changed
	elif trigger_vals == None:
		trsig = states[state_trigger].flat
		trsig = numpy.logical_and(sigdiff(trsig) != 0, trsig != 0)  # changed, and the change is not to 0
	else:
		trsig = states[state_trigger].flat
		trsig = numpy.in1d(trsig, trigger_vals)
		trsig = numpy.logical_and(trsig, sigdiff(trsig)) # within target set, and whether-within-target-set has changed
	
	trind = findrefrac(trsig, duration_samples)
	print trind
	
	
	# TODO: spawn trial segments
	
	s.opts = myargs()
	return s

def sigdiff(x, initial=0, axis=None):
	if not isinstance(x, numpy.ndarray): x = numpy.asarray(x)
	if axis == None: axis = 0
	sub0ff = [slice(None)] * len(x.shape); sub0ff[axis] = slice(None,-1)
	sub1ff = [slice(None)] * len(x.shape); sub1ff[axis] = slice(1,None)
	sub0   = [slice(None)] * len(x.shape);   sub0[axis] = slice(0,1)
	z = numpy.zeros_like(x);
	z[sub0] = x[sub0] - initial
	z[sub1ff] = x[sub1ff] - x[sub0ff]
	return z

def findrefrac(a, n, axis=0):
	b = []; prev = {}
	for coords in zip(*numpy.nonzero(a)):
		key = coords[:axis] + coords[axis+1:]
		val = coords[axis]
		p = prev.get(key, None)
		if p == None or val >= p + n: b.append(coords); prev[key] = val
	return b

	
class DotDict(dict):
	def __getattr__(self, key):
		if key in self: return self[key]
		else: raise AttributeError(key)
	def __setattr__(self, key, val):
		if key in self.__dict__: self.__dict__[key] = val
		else: self[key] = val
	def _getAttributeNames(self):
		return self.keys()
	
class listof(list):
	def __init__(self, iterable=(), type=int):
		self.__membertype = type
		for item in iterable: self.__check(item)
		list.__init__(self, iterable)
	def __check(self, x):
		if not hasattr(self, '__membertype'): return x
		if not isinstance(x, self.__membertype):
			raise TypeError('items in this list must be of type %s' % self.__membertype.__name__)
		return x
	def extend(self, iterable):
		for item in iterable: self.__check(item)
		list.extend(self, iterable)
	def __iadd__(self, iterable):
		self.extend(iterable)
		return self
	def append(self, item):
		self.__check(item)
		list.append(self, item)
	def insert(self, index, item):
		self.__check(item)
		list.insert(self, index, item)
	def __setitem__(self, index, item):
		self.__check(item)
		list.__setitem__(self, index, item)

