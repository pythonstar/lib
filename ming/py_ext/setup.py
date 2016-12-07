#
# you can use this with the fink distro of ming.. or one installed in /usr/local.. or the result of a build in the
# parent source directory.
#

from distutils.core import setup, Extension

import os

srcdir = "."
curdir = os.getcwd()

mylibs = ['ming']

if "-lz":
	mylibs.append('z')

if "-lpng":
	mylibs.append('png')

if "-lungif":
	mylibs.append("ungif")


setup(name = "mingc", version = "0.4.3",
      package_dir = {'': srcdir},
      py_modules = ['ming', 'mingc'],
      ext_modules = [Extension("_mingc", ["%s/ming_wrap.c" %srcdir],
                     include_dirs=['/usr/local/include',os.path.join(os.path.join(curdir, '..'),'src')],
                     library_dirs=['../src/.libs','/usr/local/lib/'],
                     libraries=mylibs)])

