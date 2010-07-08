#   $Id$
#   
#   This file is part of the BCPy2000 framework, a Python framework for
#   implementing modules that run on top of the BCI2000 <http://bci2000.org/>
#   platform, for the purpose of realtime biosignal processing.
# 
#   Copyright (C) 2007-10  Jeremy Hill, Thomas Schreiner,
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
#!/cygdrive/c/Python25/python.exe
# This file was generated automatically by the 'freeze' script.
import sys
args = sys.argv[1:]
if args == []: args = ['install']
if args == ['installer']:
	args = [
		'bdist_wininst',
		'--bitmap', 'installer.bmp',
		'--install-script', 'BCPy2000_postinstall.py',
	]
sys.argv = sys.argv[:1] + args

try: import setuptools
except ImportError: print "WARNING: failed to import setuptools"
from distutils.core import setup, Extension

dependencies = {
	    'numpy': '>=1.3',
	  'IPython': '>=0.8.1',
	'VisionEgg': '>=1.1',
}
setup(
	name = 'BCPy2000',
	version = '17374',
	packages = [
		'BCPy2000',
		'BCPy2000.AppTools',
		'BCPy2000.LangTools',
		'BCPy2000.SigTools',
		'BCPy2000.WavTools',
		'BCPy2000.BCI2000Tools',
		'BCPy2000.Documentation',
	],
	package_data = {
		'BCPy2000.AppTools': ['*.dll'],
		'BCPy2000.Documentation': ['*.*', 'styles/gears/*.*'],
	},
	scripts = [
		'BCPy2000_postinstall.py',
	],
	requires = [('%s (%s)' % (p,v)).replace(' ()','') for p,v in dependencies.items()],  # available in distutils from Python 2.5 onwards
	install_requires = ['%s%s' % (p,v) for p,v in dependencies.items()], # available if using setuptools,  acted on by easy_install
)
