import array
from socket import SOL_SOCKET

from eunuchs.recvmsg import recvmsg
from eunuchs.sendmsg import sendmsg
from eunuchs.socket_const import SCM_RIGHTS

def receive_fds(sock, num=1, size=1):
    """
    Receive file descriptors over a Unix domain socket.

    @param sock: socket or fd

    @type sock: socket.socket or int

    @param num: number of file descriptors to expect

    @type num: int

    @param size: how much data to read, at most; you should always
    transfer at least one byte, to detect EOF and to actually force
    C{sendmsg(2)} to do work

    @type size: int

    @return: a tuple with data read and an iterable of fds (or None)

    @rtype: tuple of (str, (iterable of int) or None)
    """
    if not isinstance(sock, int):
        sock = sock.fileno()
    (data, address, flags, ancillary) = recvmsg(
        fd=sock,
        ancillary=[
            (
                SOL_SOCKET,
                SCM_RIGHTS,
                array.array('i', num*[-1]).tostring(),
                ),
            ],
        maxsize=size,
        )

    fds = None
    for (level, type_, buf) in ancillary:
        if (level == SOL_SOCKET
            and type_ == SCM_RIGHTS):
            fds = array.array('i', buf)

    return (data, fds)

def send_fds(sock, fds, data='\n'):
    """
    Send file descriptors over a Unix domain socket.

    @param sock: socket or fd

    @type sock: socket.socket or int

    @param fds: file descriptors to send

    @type num: iterable of int

    @param data: data to transfer, to force C{sendmsg(2)} to do work

    @type data: str
    """
    if not isinstance(sock, int):
        sock = sock.fileno()
    return sendmsg(
        fd=sock,
        data=data,
        ancillary=[
            (
                SOL_SOCKET,
                SCM_RIGHTS,
                array.array('i', fds).tostring(),
                ),
            ],
        )
