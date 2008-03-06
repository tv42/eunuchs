#!/usr/bin/python

"""
See test-passwd-send.py for instructions.
"""

import errno
import os
import socket
import struct
import sys

from eunuchs.passfd import receive_fds

(SOCKPATH,) = sys.argv[1:]

s = socket.socket(
    socket.AF_UNIX,
    socket.SOCK_STREAM,
    0,
    )
try:
    os.unlink(SOCKPATH)
except OSError, e:
    if e.errno == errno.ENOENT:
        pass
    else:
        raise
s.bind(SOCKPATH)
s.listen(0)
print 'Listening...'

while True:
    print 'Waiting...'
    (client, address_info) = s.accept()
    print 'Got a connection.'

    (data, fds) = receive_fds(client, 1)
    print 'Got', repr(data)
    client.close()

    if fds is None:
        print 'Failed'
    else:
        for fd in fds:
            print 'New fd %d' % fd
            while True:
                buf = os.read(fd, 8192)
                if not buf:
                    break
                print buf
            os.close(fd)
