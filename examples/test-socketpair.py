#!/usr/bin/python
import socket, os, errno

from eunuchs.socketpair import socketpair

# set things up so the fds we get are deterministic
for fd in xrange(3,100):
    try:
        os.close(fd)
    except OSError, e:
        if e.errno == errno.EBADF:
            # wasn't open
            pass
        else:
            raise

a,b = socketpair()
assert a == 3
assert b == 4

a,b = socketpair()
assert a == 5
assert b == 6

a,b = socketpair(socket.AF_UNIX)
assert a == 7
assert b == 8

a,b = socketpair(socket.AF_UNIX, socket.SOCK_STREAM)
assert a == 9
assert b == 10

a,b = socketpair(socket.AF_UNIX, socket.SOCK_STREAM, 0)
assert a == 11
assert b == 12

a,b = socketpair(family=socket.AF_UNIX)
assert a == 13
assert b == 14

a,b = socketpair(family=socket.AF_UNIX, type=socket.SOCK_STREAM)
assert a == 15
assert b == 16

a,b = socketpair(family=socket.AF_UNIX, type=socket.SOCK_STREAM, proto=0)
assert a == 17
assert b == 18

a,b = socketpair(type=socket.SOCK_STREAM, proto=0, family=socket.AF_UNIX)
assert a == 19
assert b == 20

a,b = socketpair(type=socket.SOCK_STREAM, proto=0)
assert a == 21
assert b == 22

a,b = socketpair(family=socket.AF_UNIX, proto=0)
assert a == 23
assert b == 24

a,b = socketpair(family=socket.AF_UNIX, type=socket.SOCK_STREAM)
assert a == 25
assert b == 26

a,b = socketpair(proto=0)
assert a == 27
assert b == 28
