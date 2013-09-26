#ifndef __goofy_packet_h__
#define __goofy_packet_h__

#include <netinet/in.h>
#include "Util.h"

namespace IpPkt {

struct PktInfo {
    in_addr src;
    in_addr dst;
    in_addr local;
    uint    ifIndex;
    uchar   proto;
    uchar   ttl;
};

/* Receive an IP packet */
int recvPkt( int sock, char* buf, uint bufSz, PktInfo* info );

/* Send an IP packet */
int sendPkt( int sock, const char* buf, uint bufSz, const PktInfo* info );

};

#endif


