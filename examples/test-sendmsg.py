#!/usr/bin/python

import socket
from eunuchs.sendmsg import sendmsg
from socket import SOL_SOCKET, SO_BROADCAST, SOL_IP
IP_PKTINFO = 8 #TODO
IP_RETOPTS = 7 #TODO

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.setsockopt(SOL_SOCKET, SO_BROADCAST, 1)
s.setsockopt(SOL_IP, IP_PKTINFO, 1)
sendmsg(fd=s.fileno(),
        host='255.255.255.255',
        port=4242,
        data="foo",
        flags=0,
        ancillary=[
    (SOL_IP, IP_PKTINFO, (10, #interface index
                          '0.0.0.0',
                          '255.255.255.255',
                          )),
    #(SOL_IP, IP_RETOPTS, ''),
    ],
        )
