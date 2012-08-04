"""
Downloaded tar.gz file from http://www.csie.ntu.edu.tw/~cjlin/libsvm/ on 2012-08-04
Expanded in /tmp on OSX
$ cd libsvm-3.12/python
$ make
$ ln ../libsvm.so.2
"""

import BCPy2000.Paths
import SigTools
from svmutil import *

xtr,ytr = SigTools.demodata()
sigma, K_tr = SigTools.guesswidth(xtr, return_rbfkern=True)

xts,yts = SigTools.demodata()
K_tstr = SigTools.rbfkern(xts, xtr, sigma=sigma)

# using pre-computed kernel
problem = svm_problem(ytr, [[id+1]+row.tolist() for id,row in enumerate(K_tr)], isKernel=True)
model = svm_train( problem , svm_parameter('-t 4 -c .5 -q'))
labels, acc, vals = svm_predict(yts, [[id+1]+row.tolist() for id,row in enumerate(K_tstr)], model)

# letting LibSVM handle the RBF kernel
# (should be the same, but in practice seems to get slightly higher accuracy...)
problem = svm_problem(ytr, xtr.tolist())
model = svm_train( problem , svm_parameter('-t 2 -c .5 -g %g -q' % sigma**-2))
labels, acc, vals = svm_predict(yts, xts.tolist(), model)

# one-class SVM
problem = svm_problem(ytr[ytr>0], xtr[ytr>0].tolist())
model = svm_train( problem , svm_parameter('-s 2 -t 2 -g %g -n 0.05 -q' % sigma**-2)) # RBF version
#model = svm_train( problem , svm_parameter('-s 2 -t 0 -n 0.05 -h 0 -q')) # linear version (separate in one direction only)
labels, acc, vals = svm_predict(yts, xts.tolist(), model)
import numpy; labels = numpy.array(labels)
SigTools.scatterplot(xtr[ytr<0], 'bo', xtr[ytr>0], 'r*', xts[labels>0], 'k+', xts[labels<0], 'gx')

# NB: svmutil.py needs to be hacked in order to remove svm_predict verbosity
