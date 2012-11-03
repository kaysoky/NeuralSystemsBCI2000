# -*- coding: utf-8 -*-
# 
#   $Id$
#   
#   This file is part of the BCPy2000 framework, a Python framework for
#   implementing modules that run on top of the BCI2000 <http://bci2000.org/>
#   platform, for the purpose of realtime biosignal processing.
# 
#   Copyright (C) 2007-11  Jeremy Hill, Thomas Schreiner,
#                          Christian Puzicha, Jason Farquhar
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
	'makeparam',
	'Param', 'ParamList',
]

import numpy

def escape(s):
	if isinstance(s, Param): return '{ ' + s.make_string(verbosity=-1) + ' }'
	if isinstance(s, bool): s = int(s)
	if s == None: s = ''
	elif not isinstance(s, basestring): s = str(s)
	if len(s) == 0: return '%'
	out = ''
	for c in s:
		v = ord(c)
		if c == '%' or not 32 < v < 127: out += '%%%02x' % v
		else: out += str(c)
	return out

def unescape(s):
	# unfortunately there are two slight difference between the BCI2000 standard and urllib.unquote
	if s in ['%', '%0', '%00']: return ''  # here's one (empty string)
	out = ''
	s = list(s)
	while len(s):
		c = s.pop(0)
		if c == '%':
			c = ''.join(s[:2])
			if c.startswith('%'):  # here's the other ('%%' maps to '%')
				out += '%'
				s = s[1:]
			else:
				try: c = int(c,16)
				except: pass
				else:
					out += chr(c)
					s = s[2:]
		else:
			out += c
	return out

def unspace(x):
	x = x.strip()
	x = x.replace('\t', ' ')
	while True:
		y = x.replace('  ', ' ')
		if len(y) == len(x): break
		else: x = y
	return x

opener,closer = '{ ',' }'

def read_bracketed(x, brackets=('{ ',' }')):
	x = unspace(x)
	opener,closer = brackets
	if not x.startswith(opener):
		return '', '', x
	counter = 0
	for pos,c in enumerate(x):
		if x[pos:pos+len(opener)] == opener: counter += 1
		if x[pos:pos+len(closer)] == closer: counter -= 1
		if counter == 0: break
	cutoff = pos+len(closer)
	element = x[:cutoff]
	x = x[cutoff:].lstrip()
	stripped = element[len(opener):].lstrip()
	if stripped.endswith(closer): stripped = stripped[:-len(closer)].rstrip()
	return element,stripped,x

def read_element(x):
	head,deeper,tail = read_bracketed(x, ('{ ', ' }'))
	if not len(head): head,sep,tail = tail.partition(' ')
	return head,deeper,tail

def split_elements(x):
	elements = []
	while len(x):
		xi,deeper,x = read_element(x)
		if len(deeper): elements.append(split_elements(deeper))
		elif len(xi): elements.append(unescape(xi))
	return elements

def chomp(x, key, d=None, insist=True, lower=False, name=None):
	val = ''
	if len(x):
		val = x.pop(0)
		if d != None: d[key] = val
	elif insist:
		if name in (None,''): name=''
		else: name = ' ' + name
		raise ValueError('parameter%s definition has no %s element' % (name,key))
	if lower: val = val.lower()
	return val

def convert(val, type, name='', check_nesting=False):
	if name in (None,''): name = ''
	else: name = ' "%s"' % name
	if check_nesting and isinstance(val, list):
		return makeparam(val, parent=name)
	type = type.lower()
	if type.endswith('list'):
		val = [convert(x, type[:-len('list')], name, check_nesting=True) for x in val]
	elif type.endswith('matrix'):
		val = numpy.array([[convert(x, type[:-len('matrix')], name, check_nesting=True) for x in row] for row in val])
	elif type == 'float':
		try: val = float(val)
		except: print ('WARNING: invalid float value "%s" in parameter%s' % (str(val),name))
	elif type == 'int':
		try: val = int(val)
		except: print ('WARNING: invalid int value "%s" in parameter%s' % (str(val),name))
		
	return val

def makeparam(x, parent=None):
	d = {}
	nExpected = 1
	if isinstance(x, basestring):
		x,sep,comment = x.partition(' //')
		comment = comment.strip()
		if comment.endswith('(enumeration)'):
			c = comment[:-len('(enumeration)')].rstrip()
			c,colon,enum = c.partition(':')
			yektsrif,ecaps,tnemmoc = c.rstrip()[::-1].partition(' ')
			enum = yektsrif[::-1] + colon + enum
			comment = tnemmoc[::-1]
			d['enum'] = dict([(int(ekey.strip()),evalue.strip()) for ekey,colon,evalue in [ei.strip().partition(':') for ei in enum.split(',')]])
			
		d['comment'] = comment
		x = split_elements(x)
	if parent==None:
		location = chomp(x, 'location').split(':')
		chomp(location, 'tab', d)
		chomp(location, 'section', d, insist=False)
		chomp(location, 'filter',  d, insist=False)
	type = chomp(x, 'type', d, lower=True)
	if parent == None:
		name = chomp(x, 'name', d)
		if not name.endswith('='): raise ValueError('parameter name %s must end with =' % name)
		d['name'] = name = name.rstrip('=')
	else:
		name = parent + ' sub-parameter'
	islist = type.endswith('list')
	ismatrix = type.endswith('matrix')

	if islist:
		length = chomp(x, 'length', name=name)
		try: length = nExpected = int(length)
		except: raise ValueError('invalid length "%s" in parameter %s' % (length, name))
	elif ismatrix:
		rows = chomp(x, 'rows', name=name)
		cols = chomp(x, 'cols', name=name)
		if isinstance(rows, list): d['rlabels'],rows = rows,len(rows)
		if isinstance(cols, list): d['clabels'],cols = cols,len(cols)
		try: rows = int(rows)
		except: raise ValueError('invalid number of rows "%s" in parameter %s' % (rows, name))
		try: cols = int(cols)
		except: raise ValueError('invalid number of columns "%s" in parameter %s' % (cols, name))
		nExpected = rows * cols
		
	if len(x) < nExpected:
		raise ValueError('expected %d elements in parameter %s - found only %d' % (nExpected,name,len(x)))
	
	if islist:
		val,x = x[:nExpected],x[nExpected:]
	elif ismatrix:
		val,x = x[:nExpected],x[nExpected:]
		val = [[val.pop(0) for j in range(cols)] for i in range(rows)]
	else:
		val = x.pop(0)
	d['value'] = convert(val, type, name)
	
	x = x[-3:]
	minVal = chomp(x, 'min', insist=False, name=name)
	maxVal = chomp(x, 'max', insist=False, name=name)
	d['default'] = chomp(x, 'default', insist=False, name=name)
	d['range'] = ({'':None}.get(minVal,minVal), {'':None}.get(maxVal,maxVal))
	if parent: d['name'] = 'Unnamed'
	return Param(d)
	

class Param(object):
	
	def __init__(self, value, tab='Tab', section='', filter='', name='ParamName', type='auto', comment='', fmt=None, range=(None,None), default=None, rlabels=None, clabels=None, enum=None):
		self.tab = tab
		self.section = section
		self.filter = filter
		self.type = type
		self.name = name
		self.rlabels = rlabels
		self.clabels = clabels
		self.range = range
		self.default = default
		self.comment = comment
		self.enum = enum
		self.verbosity = 1
		self.value = value
		if isinstance(value, dict):
			x = value.pop('value')
			self.__init__(x, **value)
			
	def determine_type(self):
		x = self.value
		if isinstance(x, numpy.ndarray):
			if len(x.shape) == 0: x = x.flat[0]
			elif len(x.shape) in (1,2): x = x.tolist()
			else: raise ValueError("don't know how to deal with >2-D arrays")		
		if isinstance(x, bool): return 'bool'
		if isinstance(x, int): return 'int'
		if isinstance(x, float): return 'float'
		if isinstance(x, basestring): return 'string'
		if isinstance(x, (tuple,list)):
			if False not in [isinstance(xi, int) for xi in x]: return 'intlist'
			if False not in [isinstance(xi, (int,float)) for xi in x]: return 'floatlist'
			if False not in [isinstance(xi, (int,float,basestring)) for xi in x]: return 'list'
			if False not in [isinstance(xi, (tuple,list)) for xi in x]: return 'matrix'
		raise ValueError("don't know how to deal with this data type")
	
	def make_string(self, verbosity=1):
		type = self.type
		comment = self.comment
		if verbosity < 1: comment = ''
		if verbosity >= 0: comment = ' // ' + comment
		if type in (None, 'auto'): type = self.determine_type()
		if type == 'bool':
			type = 'int'
			if verbosity >= 0: comment = comment + ' (boolean)'
		elif type == 'int' and isinstance(self.enum, dict) and len(self.enum) and not comment.endswith('(enumeration)'):
			if verbosity > 0: comment += ' ' + (', '.join([str(k)+': '+str(v) for k,v in sorted(self.enum.items())]))
			if verbosity >= 0: comment = comment + ' (enumeration)'
		if verbosity < 0:
			location = ''
			name = ''
		else:
			name = self.name + '= '
			location = self.tab
			if self.section != None and len(self.section): location += ':' + self.section
			if self.filter  != None and len(self.filter):  location += ':' + self.filter
			location += ' '
		s = location + type + ' ' + name
		if type.endswith('list'):
			x = self.value
			# x = numpy.asarray(x).flat
			s += str(len(x))
			xstr = '    ' + ' '.join([escape(xi) for xi in x])
		elif type.endswith('matrix'):
			x = self.value
			# x = numpy.asarray(x)
			if self.rlabels == None: s += ' %d' % len(x)
			elif len(self.rlabels) != len(x):    raise ValueError("wrong number of row labels (got %d, expected %d)" % (len(self.rlabels), len(x)))
			else: s += ' { ' + ' '.join([escape(xi) for xi in self.rlabels]) + ' }' 
			if self.clabels == None: s += ' %d' % len(x[0])
			elif len(self.clabels) != len(x[0]): raise ValueError("wrong number of column labels (got %d, expected %d)" % (len(self.clabels), len(x[0])))
			else: s += ' { ' + ' '.join([escape(xi) for xi in self.clabels]) + ' }'
			xstr = '    ' + '    '.join([' '.join([escape(xi) for xi in row]) for row in x])
		else:
			xstr = escape(self.value)
		if verbosity == 0 and len(xstr) > 10: xstr = '...'
		s += ' ' + xstr
		if verbosity >= 2:
			range = self.range
			if range == None: range = (None,None)
			elif not isinstance(range, (tuple,list)): range = (0, range)
			s += '   ' + escape(self.default) + ' ' + escape(range[0]) + ' ' + escape(range[1])
		s += comment
		return s
	
	def writeto(self, file, append=False):
		if isinstance(file, basestring):
			mode = {True:'a', False:'w'}[append]
			file = open(file, mode + 't')
		if not append: file.seek(0)
		file.write(str(self)+'\n')
		
	def appendto(self, file):
		return self.writeto(file, append=True)		
	
	def __getslice__(self, s, e):
		return self.__getitem__(slice(s,e,None))

	def __getitem__(self, sub):
		def conv(self, i, x):
			if isinstance(x, (tuple, list)):
				if i == None: return x.__class__([conv(self,i,xi) for i,xi in enumerate(x)])
				else: return x.__class__([conv(self, i, xi) for xi in x])
			if i == None: i = 0
			if isinstance(x, slice): return slice(conv(self,i,x.start), conv(self,i,x.stop), conv(self,i,x.step))
			if i == 0: lab = self.rlabels; labname = 'row'
			elif i == 1: lab = self.clabels; labname = 'column'
			else: raise TypeError("too many subscripts")
			if isinstance(x, int):
				if x < 0:  x += numpy.asarray(self.value).shape[i]
				return x
			if not isinstance(x, basestring): return x
			if lab == None or x not in lab: raise ValueError("%s label '%s' not found" % (labname, x))
			return lab.index(x)
		sub = conv(self, None, sub)
		if not hasattr(sub, '__len__') or len(sub) == 1: result = numpy.asarray(self.value).__getitem__(sub)
		elif len(sub) == 2: result = numpy.asarray(numpy.asmatrix(self.value).__getitem__(sub))
		else: result = numpy.asarray(self.value).__getitem__(sub)
		if isinstance(result, numpy.ndarray):
			if len(result.shape) < 1: result = result.tolist()
			elif len(result.shape) == 1 and result.dtype.kind not in 'fib': result = result.tolist()
		return result

	def __repr__(self):
		return '<%s object at 0x%08X>: %s' % (self.__class__.__name__, id(self), self.make_string(verbosity=0))
		
	def __str__(self):
		return self.make_string(verbosity=max(1, self.verbosity))

class ParamList(list):
	def __init__(self, x=()):
		if isinstance(x, file): x = x.readlines()
		if isinstance(x, basestring): x = x.replace('\r\n', '\n').replace('\r', '\n').split('\n')
		list.__init__(self, [makeparam(str(xi)) for xi in x if len(str(xi).strip())])
	def __str__(self):
		return '\n'.join([str(xi) for xi in self])
	def __repr__(self):
		return '<%s object at 0x%08X>: [%s\n]\n' % (self.__class__.__name__, id(self), '\n  '.join(['']+[repr(xi) for xi in self]))
