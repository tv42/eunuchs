from eunuchs._tuntap import *
import struct

class TuntapPacketInfo:
    def __init__(self, packet):
        hdr, self.data = packet[:4], packet[4:]
        self.flags, self.proto = struct.unpack('!HH', hdr)

    def isPartial(self):
        return self.flags & TUN_PKT_STRIP

def makePacketInfo(flags, proto):
    return struct.pack('!HH', flags, proto)
