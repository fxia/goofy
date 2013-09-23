#ifndef __goofy_packet_h__
#define __goofy_packet_h__

#include <netinet/in.h>
#include "Util.h"

namespace Packet {

struct IpPktInfo {
    in_addr src;
    in_addr dst;
    in_addr local;
    uint    ifIndex;
    uchar   proto;
    uchar   ttl;
};

/* Receive an IP packet */
int recvPkt( int sock, char* buf, uint bufSz, IpPktInfo* info );

/* Send an IP packet */
int sendPkt( int sock, const char* buf, uint bufSz, const IpPktInfo* info );

};

#endif


