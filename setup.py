#!/usr/bin/python

#debian-section: devel

from distutils.core import setup, Extension

if __name__=='__main__':
    setup(name="python-eunuchs",
	  description="Missing manly parts of UNIX API for Python",
	  long_description="""

Eunuchs is a library of Python extension that complement the standard
libraries in parts where full support for the UNIX API (or the Linux
API) is missing.

Most of the functions wrapped by Eunuchs are low-level, dirty, but
absolutely necessary functions for real systems programming. The aim
is to have the functions added to mainstream Python libraries.

Current list of functions included:

- fchdir(2)

- recvmsg(2) and sendmsg(2), including use of cmsg(3)

- socketpair(2)

""".strip(),
	  author="Tommi Virtanen",
	  author_email="tv@debian.org",
	  url="http://eunuchs.bkbits.net/",
	  license="GNU LGPL",

	  package_dir={"": "lib"},
	  packages=[
	"eunuchs",
	],
          ext_package="eunuchs",
          ext_modules=[Extension(module, ["lib/eunuchs/%s.c" % module],
                                 extra_compile_args=['-Werror'])
                       for module in ('fchdir', 'recvmsg', 'sendmsg',
                                      'socketpair', '_tuntap')],
	  )
