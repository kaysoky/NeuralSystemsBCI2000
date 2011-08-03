#!/usr/bin/python

import os

from distutils.core import setup
import py2exe

import glob
import matplotlib

__file__ = os.path.abspath(__file__)
this_dir = os.path.dirname(__file__)

def purgeDir(dirname):
    for path, dirs, files in os.walk(dirname, topdown = False):
        for fname in files:
            try:
                os.remove(os.path.join(path, fname))
            except:
                pass
        for dir in dirs:
            try:
                os.rmdir(os.path.join(path, dir))
            except:
                pass
        try:
            os.rmdir(path)
        except:
            pass

def copy(fname, dir = this_dir):
    global this_dir
    from_path = os.path.join(dir, fname)
    to_path = os.path.join(this_dir, 'dist', fname)
    from_file = open(from_path, 'rb')
    to_file = open(to_path, 'wb')
    data = None
    while data != '':
        data = from_file.read(4096)
        to_file.write(data)

def main(argv = []):
    this_dir = os.path.dirname(__file__)
    purgeDir(os.path.join(this_dir, 'build'))
    purgeDir(os.path.join(this_dir, 'dist'))
    setup(
        name = 'Py3GUI',
        author = 'Collin Stocks',
        windows = [{'script': 'py3gui.py'}],
        zipfile = None,
        options = {
            'py2exe':{
                'includes': [
                    'numpy', 'scipy', 'cvxopt', 'cvxopt.lapack', 'matplotlib',
                    'matplotlib.pyplot', 'matplotlib.figure',
                    'matplotlib.backends.backend_tkagg', 'pylab'
                ],
                'excludes': [
                    'IPython', 'OpenGL', 'pygame', 'VisionEgg', 'Image',
                    'doctest', 'pdb', 'difflib', 'win32com', '_ssl',
                    '_gtkagg', '_agg2', '_cairo', '_cocoaagg', '_gtk',
                    '_gtkcairo', '_qt4agg'
                ],
                'optimize': 1,
                'compressed': 2,
                'ascii': True,
            }
        },
        data_files = matplotlib.get_py2exe_datafiles(),
    )
    copy('logo.gif')
    purgeDir(os.path.join(this_dir, 'dist/tcl/tk8.4/demos'))
    purgeDir(os.path.join(this_dir, 'dist/tcl/tk8.4/images'))
    purgeDir(os.path.join(this_dir, 'dist/tcl/tcl8.4/dde1.1'))
    purgeDir(os.path.join(this_dir, 'dist/tcl/tcl8.4/encoding'))
    purgeDir(os.path.join(this_dir, 'dist/tcl/tcl8.4/http1.0'))
    purgeDir(os.path.join(this_dir, 'dist/tcl/tcl8.4/http2.3'))
    purgeDir(os.path.join(this_dir, 'dist/tcl/tcl8.4/http2.5'))
    purgeDir(os.path.join(this_dir, 'dist/tcl/tcl8.4/msgcat1.0'))
    purgeDir(os.path.join(this_dir, 'dist/tcl/tcl8.4/msgcat1.3'))
    purgeDir(os.path.join(this_dir, 'dist/tcl/tcl8.4/opt0.4'))
    purgeDir(os.path.join(this_dir, 'dist/tcl/tcl8.4/tcltest1.0'))
    purgeDir(os.path.join(this_dir, 'dist/tcl/tcl8.4/tcltest2.2'))
    purgeDir(os.path.join(this_dir, 'build'))

if __name__ == '__main__':
    import sys
    sys.argv.append('py2exe')
    main(sys.argv[1:])
