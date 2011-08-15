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

import os, sys

try: __IPYTHON__
except NameError: __IPYTHON__ = get_ipython()

exec 'import os,sys' in __IPYTHON__.shell.user_ns # in case this is being run via import rather than execute

################################################################################
################################################################################

class mymagic:

	if sys.platform.lower().startswith('win'):
		if hasattr(__IPYTHON__, 'rc'): __IPYTHON__.rc.editor = 'scite'
		else: __IPYTHON__.editor = 'scite'

	############################################################################

	def makemagic(f):
		name = f.__name__
		if not name.startswith('magic_'): name = 'magic_' + name
		setattr(__IPYTHON__, name, f)
		return f
		
	############################################################################

	try: from IPython.Debugger import Tracer # IPython v 0.10 and earlier
	except ImportError: from IPython.core.debugger import Tracer # IPython v 0.11 and later
	__IPYTHON__.dbstop = Tracer()
	# calling __IPYTHON__.dbstop()  is easier than having to remember and type
	# "from IPython.Debugger import Tracer; Tracer()()" just to invoke the debugger
	# (and *much* easier than having to remember the version-specific variants of that)
	
	############################################################################
	@makemagic
	def magic_edit(fn):
		"""\
A more Matlab-like replacement for IPython's default editing behaviour:
search the path for possible matches; edit them in the background; don't
execute the result.  Set your editor with the line 'editor WHATEVER' in
ipythonrc, or by otherwise setting __IPYTHON__.rc.editor = 'WHATEVER'

Examples:

edit site.py          #  edits existing file on path with .py extension     $BLAH/Lib/site.py
edit ipythonrc        #  edits existing file on path without .py extension  $HOME/.ipython/ipythonrc
edit site             #  adds .py extension and edits existing file on path $BLAH/Lib/site.py

edit IPython/iplib.py #  edits existing file in subdirectory on path $BLAH/IPython/iplib.py
edit IPython/iplib    #  edits existing file in subdirectory on path $BLAH/IPython/iplib.py
edit IPython.iplib    #  edits existing file in package on path      $BLAH/IPython/iplib.py
edit xml              #  edits __init__.py file in existing package on path $BLAH/xml/__init__.py

edit foo.py           #  edits new file with .py extension     ./foo.py
edit foo.txt          #  edits new file with non-.py extension ./foo.txt
edit foo              #  adds .py extension and edits new file ./foo.py

edit IPython/foo.py   #  edits new file in existing subdirectory on path $BLAH/IPython/foo.py
edit IPython/foo      #  edits new file in existing subdirectory on path $BLAH/IPython/foo.py
edit IPython.foo      #  edits new file in existing subdirectory on path $BLAH/IPython/foo.py

		"""###

		# NB: one (unlikely) case is not handled:  a new file with a non-.py extension
		#     in an existing subdirectory on the path, e.g. edit IPython/foo.txt

		if hasattr(__IPYTHON__, 'rc'): editor = __IPYTHON__.rc.editor
		else: editor = __IPYTHON__.editor
		
		if sys.platform.lower().startswith('win'): editor = 'start ' + editor
		ed = lambda x: os.system(editor + ' "' + os.path.abspath(x) + '"')
		got = lambda x: os.path.isfile(os.path.abspath(x)) and (ed(x) or True)
				
		if got(fn): return
		# this allows for spaces in the path, e.g.  edit C:\Documents and Settings\me\Desktop\blah.py
		# but if the whole thing isn't an existing file, assume a space-delimited list of filenames:
		
		for original in fn.split():
			stem = original
			if stem.lower().endswith('.py'): stem = stem[:-3]
			original_withpy = stem + '.py'
			path_withoutpy = os.path.join(*stem.split('.'))
			path_withpy = path_withoutpy + '.py'
			potential_location = ''
			for p in sys.path:
				if   got(os.path.join(p, original)): break
				elif got(os.path.join(p, original_withpy)): break
				elif got(os.path.join(p, path_withpy)): break
				elif got(os.path.join(p, path_withoutpy)): break
				elif stem == original and got(os.path.join(p, path_withoutpy, '__init__.py')): break
				elif potential_location == '':
					pp = os.path.join(p, path_withoutpy)
					dd = os.path.realpath(os.path.dirname(pp))
					d  = os.path.realpath(p)
					if len(dd) > len(d) and os.path.isdir(dd): potential_location = pp
			else: # not found anywhere else, so edit as new
				if potential_location == '': potential_location = original
				fn, xtn = os.path.splitext(potential_location)
				if xtn == '': xtn = '.py'
				ed(fn + xtn)
		
	############################################################################
	@makemagic
	def magic_addpath(d=None):
		"""\
Make absolute paths out of the input(s) and append them to sys.path if they are
not already there.  Supply a space-delimited list of arguments, or one argument
enclosed in quotes (which may, in that case, contain a space).
"""###
		if d == None: dd = (os.getcwd(),)
		else:
			if (d.startswith('"') and d.endswith('"')) or (d.startswith("'") and d.endswith("'")):
				dd = (d[1:-1],)
			else:
				dd = d.split()
		for d in dd:
			d = os.path.realpath(d)
			if os.path.isdir(d):
				if not d in sys.path: sys.path.append(d)
			else:
				print 'no such directory "%s"' % d
		
	############################################################################
	@makemagic
	def magic_desk(subdir=''):
		"""\
cd to the Desktop, but keep a global record of where we were (stored in
__IPYTHON__.undeskdir) so that we can snap back with %undesk
"""###
		if sys.platform.lower().startswith('win'):
			homedrive = os.environ.get('HOMEDRIVE')
			homepath = os.environ.get('HOMEPATH')
			userprofile = os.environ.get('USERPROFILE')
			if userprofile == None:
				d = os.path.join(homedrive, homepath, 'Desktop')
			else:
				d = os.path.join(userprofile, 'Desktop')
		else:
			d = os.path.join(os.environ.get('HOME'), 'Desktop')
		if subdir != '':
			return os.path.realpath(os.path.join(d, subdir))
		here = os.path.realpath(os.curdir)
		if os.path.realpath(d) == here:
			print "already at desktop"
		else:
			__IPYTHON__.undeskdir = here
			print 'changing directory from ' + here
			print '                     to ' + d
			print 'type %undesk to change back'
			os.chdir(d)
		
	############################################################################
	@makemagic
	def magic_undesk(arg):
		"""\
Undo a previous call to %desk
"""###
		d = getattr(__IPYTHON__, 'undeskdir', None)
		if d == None:
			print 'no global record of previous call to %desk'
		else:
			print 'changing directory from ' + os.path.realpath(os.curdir)
			print '                back to ' + d
			os.chdir(d)
	
	
	############################################################################
	@makemagic
	def magic_easy_install(d=None):
		"""\
Run the easy_install command from IPython, invoking the setuptools script
Scripts/easy_install-script.py (on Windows) or easy_install (on anything else).
Particularly useful on Windows, where IPython may well be the best command-line
you have.
"""###
		def readpth(pth):
			sitedir = os.path.split(pth)[0] # assume easy-install.pth is directly in the site-packages dir
			lines = [x.strip() for x in open(pth, 'rt').readlines()]
			return  [os.path.realpath(os.path.join(sitedir, x)) for x in lines if not x.startswith('import') and not x.startswith('#')]
			# assume a line is a relative path unless it begins with # or import

		def isbelow(x, p):
			x,p = os.path.realpath(x), os.path.realpath(p)
			return x == p or x.startswith(os.path.join(p, ''))

		pth = [os.path.join(os.path.realpath(x), 'easy-install.pth') for x in sys.path] # assume 'easy-install.pth' is on the path
		pth = ([x for x in pth if os.path.isfile(x)]+[None])[0]
		if pth != None: oldpaths = readpth(pth)
		
		############ This is the part that actually runs easy_install.
		############ The rest, before and after, is to attempt to keep the session going with intelligently updates to
		############ sys.path and sys.modules (still won't work for upgrades to IPython and some messy others like simplejson).
		home = os.environ.get('PYTHONHOME')
		if sys.platform.lower().startswith('win'):
			if home == None: home = os.path.dirname(sys.executable)
			__IPYTHON__.magic_run('"' + os.path.join(home, 'Scripts', 'easy_install-script.py') + '" ' + d)
		else:
			# TODO: need to set $PYTHONHOME for system call using env?  If so, how to fill it in if home==None?
			os.system('"' + sys.executable + '" "`which easy_install`" ' + d)
		############
		
		if pth == None:
			print "restart this python session to take advantage of the new package"
			return
			
		newpaths = readpth(pth)
		addpaths = [x for x in newpaths if x not in oldpaths]
		gone     = [x for x in oldpaths if x not in newpaths]
		rmpaths  = []
		for x in sys.path:
			if not len(x): continue
			for stem in gone:
				if isbelow(x,stem):
					rmpaths.append(x)
					break
		stripped = [x for x in sys.path if x not in rmpaths]
		for i in range(len(stripped)):
			if len(stripped[i]): stripped[i] = os.path.realpath(stripped[i])
		sys.path[:] = stripped + addpaths
		if len(addpaths): print('\n  '.join(['Added to sys.path:'] + map(repr, addpaths)))
		if len(rmpaths):  print('\n  '.join(['Removed from sys.path:'] + map(repr, rmpaths)))
		rmmod = []
		for k,v in sys.modules.items():
			v = getattr(v, '__file__', None)
			if not isinstance(v, basestring): continue
			for p in rmpaths:
				if isbelow(v, p): rmmod.append(k); break
		rmmod = [sys.modules.pop(k) for k in sorted(rmmod)]
		if len(rmmod):  print('\n  '.join(['Removed from sys.modules:'] + map(repr, rmmod)))
		
	############################################################################
	@makemagic
	def magic_reimport(dd):
		"""\
The syntax

    %reimport foo, bar.*

is a shortcut for the following:

    import foo; foo = reload(foo)
    import bar; bar = reload(bar); from bar import *
"""###
		ipython = __IPYTHON__.shell.user_ns
		for d in dd.replace(',', ' ').split(' '):
			if len(d):
				bare = d.endswith('.*')
				if bare: d = d[:-2]
				exec 'import xx; xx = reload(xx)'.replace('xx', d) in ipython
				if bare: exec 'from xx import *'.replace('xx', d) in ipython
		
	############################################################################
	@makemagic
	def magic_njh(*pargs):
		"""\
Imports a number of packages, including the BCPy2000 sub-packages WavTools and
SigTools, but also copy, struct, ctypes, numpy, scipy and matplotlib and pylab
(the latter in interactive mode). Also define %pp as an ipython command-line
shortcut to SigTools.summarize, to give a quick look at object attributes---
especially useful for numpy arrays.
"""###
		ipython = __IPYTHON__.shell.user_ns
		exec 'import copy,struct,ctypes,time,numpy,scipy' in ipython
		try:
			exec 'import WavTools,SigTools' in ipython
		except ImportError:	
			exec 'import BCPy2000.Paths'    in ipython
			exec 'import WavTools,SigTools' in ipython
			
		def magic_pp(name=''):
			if name == None: return
			exec 'print SigTools.summarize(' + name + ')' in ipython
		__IPYTHON__.magic_pp = magic_pp
		__IPYTHON__.magic_loadpylab()
		
	############################################################################
	@makemagic
	def magic_loadpylab(d=''):
		"""\
The syntax

	%loadpylab

imports matplotlib and pylab into the IPython workspace, ensuring that, if matplotlib
has not previously been loaded, it is loaded in interactive mode.

	%loadpylab *

is the same, but additionally imports the requested symbols from pylab, directly into
the workspace.

"""###
		import sys
		ipython = __IPYTHON__.shell.user_ns
		try: import matplotlib
		except ImportError: print "WARNING: failed to import matplotlib"
		else:
			if not 'matplotlib.backends' in sys.modules: matplotlib.interactive(True)
			try: exec 'import matplotlib, pylab' in ipython
			except ImportError: print "WARNING: failed to import pylab"
			if len(d): exec ('from pylab import '+d) in ipython

################################################################################
################################################################################

del mymagic # Remove class definition from namespace again:  side-effects of the @makemagic decorator
            # will have already done the work (shortlisted for the Sneaky Hack Of The Year award 2009)
