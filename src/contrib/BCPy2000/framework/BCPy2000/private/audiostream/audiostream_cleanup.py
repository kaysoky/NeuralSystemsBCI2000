import os,sys,numpy
import BCPy2000.Paths
from BCI2000Tools.FileReader import bcistream

dataroot = 'C:/BCPy2000-FullMonty254-20100708/BCI2000/data'
datadirs = sorted([os.path.join(dataroot, x) for x in os.listdir(dataroot) if os.path.isdir(os.path.join(dataroot,x)) and x.startswith('20') and 'XXXX' not in x])

patterns = (getattr(sys, 'argv', []))[1:]
if len(patterns): datadirs = [d for d in datadirs if True in [x in d for x in patterns]]
print '\n'.join(datadirs)

dd = {}
datfiles = []
for d in datadirs:
	dd[d] = sorted([os.path.join(d, x) for x in os.listdir(d) if x.lower().endswith('.dat')])
	datfiles += dd[d]

for dfile in datfiles:
	stem,xtn = os.path.splitext(dfile)
	pfile = stem + '_predictions.txt'
	rfile = stem + '_responses.txt'
	
	x,st=None,None
	if not os.path.isfile(pfile):
		print "missing predictions:",pfile
		if x == None: x,st = bcistream(dfile).decode('all')
		t = numpy.nonzero(numpy.diff(st['PredictedStream'].flat)>0)[0]+1
		predicted = st['PredictedStream'].flat[t]
		target = st['TargetStream'].flat[t]
		out = open(pfile, 'w')
		out.write('[\n')
		for t,p in zip(target,predicted): out.write('\t[%d,%d],\n' % (p, t)) # that's right: predicted in first col, target in second
		out.write(']\n')
		out.close()
		
	if not os.path.isfile(rfile):
		print "missing responses:",rfile
		if x == None: x,st = bcistream(dfile).decode('all')
		t = numpy.nonzero(numpy.diff(st['Response'].flat)>0)[0]+1
		response = st['Response'].flat[t]
		correctResponse = st['CorrectResponse'].flat[t]
		out = open(rfile, 'w')
		out.write('[\n')
		for c,r in zip(correctResponse,response): out.write('\t[%d,%d],\n' % (r, c)) # that's right: given response in first col, correct response in second
		out.write(']\n')
		out.close()
