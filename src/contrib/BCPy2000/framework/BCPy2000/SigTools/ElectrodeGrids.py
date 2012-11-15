#!/usr/bin/env python
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

__doc__ = """
Try this:

  from $0 import GridSet, ExampleGrids
  g = Grids(ExampleGrids)
  print g
  print g.ChannelNames()
  print g.BipolarSpatialFilter()
  
  g.ChannelNames('>foo.prm')
  g.BipolarSpatialFilter('>>foo.prm')
"""###

standalone_doc = """
  $0 --help            print this help
  $0 --example         print an annotated example of the input format
  $0 --clean-example   print the example without the annotations

  $0 [--order=XX] [--sep=YY] [--prefix=ZZ] [--alphabetize] [--visualize=VV] FILENAME
  
  Process the file specified by FILENAME (if given) or stdin (if not) and produce a parameter
  fragment containing the BCI2000 parameters ChannelNames, SpatialFilter, SpatialFilterType and
  (optionally) VisualizeSpatialFilter.  Options are as follows:
"""###

ExampleGrids = """
FG:    # Strings followed by a colons denote the (abbreviated) names of electrode grids.

    .  .  .  .  .  .  .  1     #  Each electrode is marked by a number or by a '.' or 'o' character.
    .  .  .  .  .  .  .  2     #  The numbering of the unlabelled electrodes is inferred from
    .  .  .  .  .  .  .  .     #  consecutive numbered pairs (must supply at least one such pair).
    .  .  .  .  .  .  .  .
    .  .  .  .  .  .  .  .     #  The number of spaces between columns is unimportant provided it
    .  .  .  .  .  .  .  .     #  remains consistent. Ditto the number of blank lines between rows.
    .  .  .  .  .  .  .  X
    .  .  .  .  .  .  .  X     #  An X marks a "lost number" - perhaps an electrode that was
                               #  originally numbered but is now absent, having been cut off.

TG:
                               #  As you have probably figured out by now, a '#' denotes a comment.
    1  2  .  .  .  .  .  .
    .  .  .  .  .  .  .  .
    .  .  .  .  .  .  .  .
    .  .  .  .                 #  In contrast to an X,  a blank space corresponds to an area that
    .  .  .  .                 #  is free of electrodes by design - so there are no lost numbers.
    .  .  .  .

# If there are discontinuities in numbering that cannot be handled by X's, the best bet is to
# create separate grids.

"""###    

universaldoc = """

  order        is a list of strings, or a comma-delimited string, specifying grid names - for
               example, 'TG,FG'. By default, when order=None, grids come out in the order in
               which they were specified in the original text input.

  prefix       can be used to override the grid name

  sep          is the string used to glue the grid name to the electrode number

  precision    is the number of digits to use to render each electrode number.  If left at None,
               a precision is automatically chosen for all grids to accommodate the grid with
               the largest number of electrodes.

  prm          may be:
                 - True, in which case return a BCI2000-formatted string
                 - False, in which case return objects
                 - a filename, optionally beginning with '>', in which case write the BCI2000
                   parameters to that file, overwriting it completely, and return nothing
                 - a filename beginning with '>>', in which case append rather than overwriting

  numsort      specifies whether to sort the electrodes of each grid in increasing numerical
               order, (False would mean they are reported in spatial order top-down, left to right)

  alphabetize  Older versions of SpatialFilter can only order their output channels alphabetically,
               thereby destroying the spatial order of the bipolar montage. Set the alphabetize
               option if you want to work around this: an alphabetic prefix will be added to each
               output channel label to ensure that the spatial order is preserved.

  visualize    if given, outputs a VisualizeSpatialFilter parameter with the given value.
"""###

__all__ = [
	'GridSet', 'ExampleGrids',
]

import os,sys
import math
import itertools
import inspect
import getopt

execname = os.path.split(__file__)[1]
if execname.endswith('.pyc'): execname = execname[:-1]
universaldoc = dict([(x.split()[0], '\n\n'+x.rstrip()+'\n') for x in universaldoc.strip('\n').split('\n\n')])
standalone_doc = standalone_doc.replace('$0', execname)
for arg in 'order sep prefix alphabetize visualize'.split(): standalone_doc = standalone_doc.rstrip() + universaldoc[arg]
__doc__ = __doc__.replace('$0', os.path.splitext(execname)[0]).rstrip()

def adddoc(func):
	for arg in inspect.getargs(func.func_code).args:
		if arg not in universaldoc: continue
		if func.__doc__ == None: func.__doc__ = ''
		func.__doc__ = func.__doc__.rstrip() + universaldoc[arg]
	return func

class ElectrodeCoordinate(object):
	def __init__(self, gridName=None, localRowIndex=None, localColumnIndex=None, localNumber=None, globalNumber=None, globalX=None, globalY=None):
		self.gridName = gridName
		self.localRowIndex = localRowIndex
		self.localColumnIndex = localColumnIndex
		self.localNumber = localNumber
		self.globalNumber = globalNumber
		self.globalX = globalX
		self.globalY = globalY
	def name(self, prefix=None, sep='', precision=3, escape=False):
		if prefix == None: prefix = self.gridName
		if len(prefix): prefix += sep
		x = prefix + (('%0' + str(precision) + 'd') % self.localNumber)
		if escape: x = x.replace(' ', '%20')
		return x

def CleanInput(input):
	isseq = isinstance(input, (tuple,list))
	if isseq: lines = input
	else: lines = input.split('\n')
	out = ''
	commentchar = '#'
	starting = True
	for line in lines:
		if commentchar in line: line = line[:line.index(commentchar)]
		line = line.rstrip()
		if ':' in line:
			out = out.rstrip() + '\n\n'
			if not line.endswith(':'):
				br = line.index(':') + 1
				out += line[:br] + '\n'
				line = (' ' * br + line[br:]).rstrip()
				starting = True
		if starting and len(line)==0: continue
		starting = line.endswith(':')
		out += line + '\n'
	out = out.lstrip('\n').rstrip() + '\n'			
	while ':\n\n' in out: out = out.replace(':\n\n', ':\n')
	if isseq: out = out.split('\n')
	return out
	
def ParseGrids(input):

	if isinstance(input, basestring) and ('\n' in input or '\r' in input or ':' in input):
		input = input.replace('\r\n', '\n').replace('\r', '\n').split('\n')
		
	if isinstance(input, (tuple,list)):
		lines = input
	else:
		if isinstance(input, basestring): input = open(input)
		if not hasattr(input, 'readlines'): raise TypeError('do not know how to interpret input of type %s' % input.__class__.__name__)
		lines = input.readlines()

	out = []; names = set()
	grid = []; gridname = 'A';
	lines = CleanInput(lines)
		
	for iline,line in enumerate(list(lines) + [':']):
		line = line.rstrip()
		if line.endswith(':'):
			if len(grid):
				if gridname in names: raise ValueError('multiple grids with same name "%s"' % gridname)
				names.add(gridname)
				out.append(ParseOneGrid(grid, gridname)) # if each grid is a separate sublist within gs $$$
				# out += ParseOneGrid(grid, gridname)  # if all electrodes are together in one flat list $$$
			gridname = line[:-1].strip()
			grid = []
		elif len(line.strip()) == 0:
			if len(grid): grid.append('')
		else:
			grid.append(line)	
	return out

def ParseOneGrid(lines, gridname):
	rows = []; cols = []; vals = []; keep = []; word = ''
	for iline,line in enumerate(lines):
		for ichar,char in enumerate(' ' + line + ' '):
			if char.isspace():
				if len(word):
					rows.append(float(iline))
					cols.append(ichar - float(len(word)+1)/2.0)
					if   word in ['x', 'X']: vals.append(None); keep.append(False)
					elif word in ['o', 'O', '.']: vals.append(None); keep.append(True)
					else: vals.append(int(word)); keep.append(True)
					word = ''
			else:
				word += char
				
	def regularize(coords):
		c = sorted(list(set(coords)))
		c = [c[i+1]-c[i] for i in range(len(c)-1)]
		if len(c) == 0: return coords
		mid = (len(c)-1)/2.0
		stride = (c[int(math.floor(mid))] + c[int(math.ceil(mid))]) / 2.0
		stride = math.ceil(stride)
		return [int(round((x-min(coords))/stride)) for x in coords]

	rows = regularize(rows)
	cols = regularize(cols)
		
	grid  = [[v,rows[i],cols[i],keep[i]] for i,v in enumerate(vals)]
	known = [[v,rows[i],cols[i]] for i,v in enumerate(vals) if v != None]
	incomplete = len(known) < len(grid)
	
	if len(known) == 0: raise ValueError("no numbers are marked in %s - need at least 2" % (gridname))
	if len(known) == 1: raise ValueError("only one number is marked in %s - need at least 2" % (gridname))
	startval,startrow,startcol = known[0]
	lastval,lastrow,lastcol = known[1]
	if lastcol==startcol: # working up/down within the same column, i.e. in column-major order, which means row-index varies faster, which means column index is more significant in sorting
		if incomplete and abs(lastval-startval) != abs(lastrow-startrow): raise ValueError("in %s, numbers %d and %d are positioned %d rows apart (should be %d)" % (gridname, startval, lastval, abs(lastrow-startrow), abs(lastval-startval)))
		verticalDirection = (lastval-startval) * (lastrow-startrow)
		if verticalDirection == 0: verticalDirection = 1
		horizontalDirection = ((max(cols)-startcol) - (startcol-min(cols))) * ((len(grid)-startval) - (startval-1))
		if horizontalDirection == 0: horizontalDirection = 1
		grid = sorted(grid, cmp=lambda x,y: cmp([horizontalDirection*x[2],verticalDirection*x[1]],  [horizontalDirection*y[2],verticalDirection*y[1]]) )
	elif lastrow==startrow: # working up/down within the same row, i.e. in row-major order, which means column-index varies faster, which means row index is more significant in sorting
		if incomplete and abs(lastval-startval) != abs(lastcol-startcol): raise ValueError("in %s, numbers %d and %d are positioned %d columns apart (should be %d)" % (gridname, startval, lastval, abs(lastcol-startcol), abs(lastval-startval)))
		horizontalDirection = (lastval-startval) * (lastcol-startcol)
		if horizontalDirection == 0: horizontalDirection = 1
		verticalDirection = ((max(rows)-startrow) - (startrow-min(rows))) * ((len(grid)-startval) - (startval-1))
		if verticalDirection == 0: verticalDirection = 1
		grid = sorted(grid, cmp=lambda x,y: cmp([verticalDirection*x[1],horizontalDirection*x[2]],  [verticalDirection*y[1],horizontalDirection*y[2]]) )
	else:
		raise ValueError("numbers %d and %d in %s do not share the same row or the same column" % (startval, lastval, gridname))
	
	prevval = None
	for entry in grid:
		if entry[0] == None and prevval != None: entry[0] = prevval + 1
		prevval = entry[0]
	prevval = None
	for entry in grid[::-1]:
		if entry[0] == None and prevval != None: entry[0] = prevval - 1
		prevval = entry[0]
	
	grid = sorted(grid, cmp=lambda x,y: cmp([x[1],x[2]],  [y[1],y[2]]) )
	return [ElectrodeCoordinate(gridName=gridname, localNumber=x[0], localRowIndex=x[1], localColumnIndex=x[2]) for x in grid if x[-1]]  # select only those entries where keep[i] == True


def GetGridNames(gs):
	return [g[0].gridName for g in gs] # if each grid is a separate sublist within gs $$$
	#return [key for key,val in itertools.groupby([x.gridName for x in gs])] # if all electrodes are together in one flat list $$$

def GetGrid(gs, gridname, numsort=True):
	subset = gs[GetGridNames(gs).index(gridname)] # if each grid is a separate sublist within gs $$$
	#subset = [x for x in gs if x.gridName.lower() == gridname.lower()] # if all electrodes are together in one flat list $$$
	if numsort: subset = [x for n,x in sorted([(x.localNumber,x) for x in subset])]
	return subset
	
def EachGrid(gs, order=None, numsort=True):
	if isinstance(order, basestring): order = order.replace(',',' ').replace(';',' ').split()
	if order == None: order = GetGridNames(gs)
	return [(name, GetGrid(gs, name, numsort=numsort)) for name in order]

def NumericalPrecision(gs):
	return max([int(math.ceil(math.log10(1.0 + max([x.localNumber for x in grid])))) for name,grid in EachGrid(gs)])



@adddoc
def ReportGrids(gs, order=None, sep='', precision=None):
	"""
	Returns a full ASCII representation of each grid, with all the numbers filled in.
	"""###
	s = ''
	if precision==None: precision = NumericalPrecision(gs)
	blank = ' ' * precision
	for gridname, subset in EachGrid(gs, order=order):
		di = dict([((x.localRowIndex, x.localColumnIndex), x.name(prefix='', sep=sep, precision=precision)) for x in subset])
		maxrow = max([x.localRowIndex for x in subset])
		maxcol = max([x.localColumnIndex for x in subset])
		s += gridname + ':\n'
		for row in range(int(maxrow+1)):
			s += '  '
			s += '  '.join([di.get((row,col), blank) for col in range(int(maxcol+1))])
			s += '\n'
		s += '\n'
	return s
	
@adddoc
def PlotGrids(gs, order=None, drawnow=True):
	"""
	Plot the grids, each on a separate subplot if there are more than one, to be sure of the
	row and column coordinates inferred for each numberred electrode.
	
	NB: requires installation of the matplotlib python package (including the pylab module)
		for plotting.
	"""###

	import pylab
	each = EachGrid(gs, order=order)
	ngrids = len(each)
	for rows in range(int(ngrids**0.5), 0, -1):
		if ngrids % rows == 0: cols = ngrids / rows; break
	else: rows = int(ngrids**0.5); cols = int(math.ceil(ngrids/float(rows)))
	if ngrids != 1: pylab.clf()
	for i,(gridname,g) in enumerate(each):
		if ngrids != 1: pylab.subplot(rows, cols, i+1)
		pylab.cla()
		xcoords,ycoords = [],[]
		for trode in g:
			row,col,val = trode.localRowIndex,trode.localColumnIndex,trode.localNumber
			xcoords.append(col); ycoords.append(row)
			pylab.text(col, row, str(val), horizontalalignment='center', verticalalignment='center')
		pylab.xlim(min(xcoords)-1, max(xcoords)+1)
		pylab.ylim(max(ycoords)+1, min(ycoords)-1)
		pylab.grid('on')
		pylab.title(gridname)
	if drawnow:
		pylab.draw()
		pylab.show()

def WriteFile(content, f):
	if isinstance(content, (tuple,list)): txt = '\n'.join(content)
	else: txt = content
	if isinstance(f, basestring):
		if f.startswith('>>'): mode = 'at'
		else: mode = 'wt'
		f = open(f.lstrip('>'), mode)
	if isinstance(f, file): f.write(txt+'\n'); return None
	else: return content

@adddoc
def ChannelNames(gs, prm=True, order=None, precision=None, prefix=None, sep='', numsort=True):
	"""
	Return (or save to file) a BCI2000 ChannelNames parameter based on the grid(s) gs.
	"""###
	if precision==None: precision = NumericalPrecision(gs)	
	out = []
	for gridname, subset in EachGrid(gs, order=order, numsort=numsort):
		subset = [x.name(prefix=prefix, sep=sep, precision=precision) for x in subset]
		out += subset
	if prm:
		out = "Source list ChannelNames= "   + str(len(out)) + '    ' +  ' '.join([x.replace(' ', '%20') for x in out])
		out = WriteFile(out, prm)
	return out

@adddoc
def BipolarSpatialFilter(gs, prm=True, order=None, precision=None, prefix=None, sep='', alphabetize=False):
	"""
	Return (or save to file) BCI2000 SpatialFilter and SpatialFilterType parameters that
	"""###
	if precision == None: precision = NumericalPrecision(gs)
	out = []
	for gridname,subset in EachGrid(gs, order=order):
		thisgrid = []
		for neg in subset:
			for pos in subset:
				pr,pc,nr,nc = pos.localRowIndex,pos.localColumnIndex,neg.localRowIndex,neg.localColumnIndex
				# loc = (pos.localNumber+neg.localNumber)/2.0  # this would be a sort of numsort for bipolar interstices, but it doesn't work very well
				loc = ((pr+nr)/2.0, (pc+nc)/2.0)
				if pr == nr + 1 and pc == nc:
					thisgrid.append((loc,neg,pos))
				if pc == nc + 1 and pr == nr:
					thisgrid.append((loc,neg,pos))
				# TODO:  This only works for square grids.  Could use a distance-is-as-small-as-they-get-except-0-when-rounded criterion instead: that would capture hex grids too.
		out += [(neg,pos) for loc,neg,pos in sorted(thisgrid)]
	globalPrecision = int(math.ceil(math.log10(1.0 + len(out))))
	globalFmt = '%0' + str(globalPrecision) + 'd'
	globalIndex = 0
	if prm:
		mat = []
		for neg,pos in out:
			neg = neg.name(prefix=prefix, sep=sep, precision=precision, escape=True)
			pos = pos.name(prefix=prefix, sep=sep, precision=precision, escape=True)
			diff = pos + '-' + neg
			if alphabetize:
				globalIndex += 1
				op = (globalFmt % globalIndex)
				op = ''.join([chr(ord(c)+ord('a')-ord('0')) for c in op])
				diff = op + '=' + diff
			mat.append(neg + ' ' + diff + ' -1')
			mat.append(pos + ' ' + diff + '  1')
		out = [
			"Filtering int SpatialFilterType= 2 // spatial filter type 0: none, 1: full matrix, 2: sparse matrix, 3: common average reference (CAR) (enumeration)",
			"Filtering matrix SpatialFilter= "   + str(len(mat)) + ' { In Out Wt }  ' +  '    '.join(mat) + ' // ',
		]
		out = '\n'.join(out)
		out = WriteFile(out, prm)

	return out

def test(*pargs, **kwargs):
	d1 = ParseGrids(*pargs, **kwargs)
	r1 = ReportGrids(d1)
	d2 = ParseGrids(r1)
	r2 = ReportGrids(d2)
	print r2
	return r1 == r2
	
class smartlist(list):
	# nice repr showing class and id
	def __repr__(self): return '<%s.%s object at 0x%08X>: [%s\n]\n' % (self.__module__,self.__class__.__name__,id(self),'\n  '.join(['']+[('\n'+repr(x)).replace('\n', '\n  ').lstrip('\n').rstrip()+',' for x in self]))
	# boilerplate stuff required just to make a list subclass keep its class identity following common operations
	def __getslice__(self, *pargs, **kwargs):
		result = list.__getslice__(self, *pargs, **kwargs)
		if isinstance(result, list) and not isinstance(result, self.__class__): result = self.__class__(result)
		return result
	def __iadd__(self, other):
		if not isinstance(other, self.__class__): other = self.__class__(other)
		list.__iadd__(self, other); return self
	def __imul__(self, fac): list.__imul__(self, fac); return self
	def __add__(self, other): result = self.__class__(self); result += other; return result
	def __mul__(self, other): result = self.__class__(self); result *= other; return result
	def __rmul__(self, other): return self.__mul__(other)

class Grid(smartlist): pass
class GridSet(smartlist):
	__doc__ = __doc__
	def __init__(self, arg=None):
		if not isinstance(arg, list): arg = ParseGrids(arg)
		arg = [isinstance(x, Grid) and x or Grid(x) for x in arg] # if each grid is a separate sublist within gs $$$
		list.__init__(self, arg)
	def __str__(self): return self.report()	
	ChannelNames = ChannelNames
	BipolarSpatialFilter = BipolarSpatialFilter
	report = ReportGrids
	def ChannelSet(self, **kwargs):
		from Electrodes import ChannelSet
		return ChannelSet(self.ChannelNames(prm=False, **kwargs))

__doc__ += '\n\nThis module can also be used as a standalone program:\n\n' + standalone_doc.lstrip('\n')
	
if __name__ == '__main__':
	args = getattr(sys, 'argv', [])[1:]
	try:
		opts,args = getopt.getopt(args, '', ['help', 'example', 'clean-example', 'order=', 'sep=', 'prefix=', 'alphabetize', 'visualize='])
	except Exception,e:
		sys.stderr.write(str(e)+'\n')
		exit(1)

	opts = dict([(k.lstrip('-'),v) for k,v in opts])
	alphabetize = opts.pop('alphabetize', None) != None
	visualize = opts.pop('visualize', '')
	if len(args): input = args.pop(0)
	else: input = sys.stdin
	
	if len(args):
		sys.stderr.write('too many input arguments\n')
		exit(1)
		
	if 'help' in opts:
		print standalone_doc
	elif 'example' in opts:
		print ExampleGrids
	elif 'clean-example' in opts:
		print CleanInput(ExampleGrids)
	else:
		d = GridSet(input)
		print d.ChannelNames(**opts)
		print d.BipolarSpatialFilter(alphabetize=alphabetize, **opts)
		if len(visualize.strip()): print 'Visualization int VisualizeSpatialFilter= ' + str(visualize)
