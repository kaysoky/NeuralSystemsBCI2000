#!/usr/bin/python

import sys
from FileReader import unescape, DecodeUnits, DatFileError

def sublists(list):
	def _sublists(list):
		ret = []
		popped = None
		while len(list):
			popped = list.pop(0)
			if popped in ['[', '{']:
				popped = _sublists(list)
			elif popped in ['}', ']']:
				break
			ret.append(popped)
		return ret
	return _sublists(list[:])

def decode_category(categorystr):
	category = [unescape(x) for x in categorystr.split(':')]
	category += [''] * (3 - len(category))
	if len(category) > 3:
		# this shouldn't happen, but some modules seem to register parameters
		# with the string '::' inside one of the category elements. Let's
		# assume this only happens in the third element.
		category = category[:2] + [':'.join(category[2:])]
	return category

def parse_shortform(data):
	print data
	def decode_units(values, valtype):
		scaled = []
		for val in values:
			if isinstance(val, list):
				scaled.append(val)
			else:
				unscaled, units, scaled_val = DecodeUnits(val, valtype)
				scaled.append(scaled_val if scaled_val != None else unscaled)
		return scaled
	def fill_matrix(shape, data):
		if shape == (1,):
			print data
			a = data.pop(0)
			print a
			return a
		if len(shape) == 1:
			ret = []
			for i in range(shape[0]):
				ret.append(data.pop(0))
			return ret
		ret = []
		for i in range(shape[0]):
			ret.append(fill_matrix(shape[1:], data))
		return ret
	datatype = data.pop(0).strip()
	if datatype.endswith('list'):
		valtype = datatype[:-4]
		dim = data.pop(0)
		if isinstance(dim, list):
			labels = dim
		else:
			labels = range(1, int(dim) + 1)
		shape = (len(labels),)
		dimlabels = (labels,)
	elif datatype.endswith('matrix'):
		valtype = datatype[:-6]
		rows = data.pop(0)
		if isinstance(rows, list):
			rowlabels = rows
		else:
			rowlabels = range(1, int(rows) + 1)
		cols = data.pop(0)
		if isinstance(cols, list):
			collabels = cols
		else:
			collabels = range(1, int(cols) + 1)
		shape = (len(rowlabels), len(collabels))
		dimlabels = (rowlabels, collabels)
	else:
		valtype = datatype
		shape = (1,)
		dimlabels = ([1],)
	valtype = {'float':float, 'int':int, '':object, 'string':str,
		'variant':object}.get(valtype, object)
	for i in range(len(data)):
		if isinstance(data[i], list):
			data[i] = parse_shortform(data[i])
	values = fill_matrix(shape, data)
	scaled = decode_units(values, valtype)
	length = 1
	for i in shape:
		length *= i
	return {
		'type'        : datatype,
		'valtype'     : valtype,
		'shape'       : shape,
		'len'         : length,
		'val'         : values,
		'scaled'      : scaled,
		'dimlabels'   : dimlabels,
	}

def ParseParam2(param):
	param = param.strip().split('//', 1)
	comment = ''
	if len(param) > 1:
		comment = param[1].strip()

	param = param[0].split()
	category = decode_category(param.pop(0))
	param = [unescape(x) for x in param]
	datatype = param.pop(0)
	name = param.pop(0).rstrip('=')
	rec = {
		'name': name, 'comment': comment, 'category': category,
		'defaultVal': '', 'minVal': '', 'maxVal': '',
	}
	param = sublists(param)
	param = [datatype] + param
	rec.update(parse_shortform(param))
	if len(param):
		param = [datatype] + param
		rec['defaultVal'] = parse_shortform(param)['val']
	if len(param):
		param = [datatype] + param
		rec['minVal'] = parse_shortform(param)['val']
	if len(param):
		param = [datatype] + param
		rec['maxVal'] = parse_shortform(param)['val']
	return rec
