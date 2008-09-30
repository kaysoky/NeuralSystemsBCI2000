#   $Id$
#   
#   This file is part of the BCPy2000 framework, a Python framework for
#   implementing modules that run on top of the BCI2000 <http://bci2000.org/>
#   platform, for the purpose of realtime biosignal processing.
# 
#   Copyright (C) 2007-8  Thomas Schreiner, Jeremy Hill
#                         Christian Puzicha, Jason Farquhar
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
#!/cygdrive/c/Python24/python.exe
# This file was generated automatically by the 'freeze' script.
import sys
args = sys.argv[1:]
if args == []: args = ['install']
if args == ['installer']:
	args = [
		'bdist_wininst',
		'--bitmap', 'installer.bmp',
		'--install-script', 'post_install.py',
	]
sys.argv = sys.argv[:1] + args

from distutils.core import setup
setup(
	name = 'BCPy2000',
	version = '1.0',
	packages = [
		'BCPy2000',
		'BCPy2000.AppTools',
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
		'post_install.py',
	],
	#requires = ['VisionEgg (>=1.0)', 'numpy (>=1.0)', 'IPython (>=0.8.1)'],
)
