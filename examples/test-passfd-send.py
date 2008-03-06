#!/usr/bin/python

"""
Demo to pass a file descriptor from one process to another.

Do something like this:

 - in one terminal, run and leave running::

	./test-passfd-receive.py sock

 - in another terminal, run::

	echo hello | ./test-passfd-send.py sock

You should now see that "hello" output from the first terminal.
"""

import socket
import struct
import sys

from eunuchs.passfd import send_fds

(SOCKPATH,) = sys.argv[1:]

s = socket.socket(
    socket.AF_UNIX,
    socket.SOCK_STREAM,
    0,
    )
s.connect(SOCKPATH)
send_fds(s, [0])
s.close()
