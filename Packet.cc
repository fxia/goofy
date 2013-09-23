#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/ip.h>

#include "Packet.h"

using namespace Packet;

int
recvPkt( int sock, char* buffer, uint bufSz, IpPktInfo* pktInfo )
{
    assert( buffer && ( bufSz > 0 ) );
    
    unsigned char name[32];
    unsigned char ctrlBuffer[1024];
    
    struct iovec iovec = { buffer, bufSz };
    struct msghdr msg;

    memset( name, 0, sizeof(name) );
    msg.msg_name = name;
    msg.msg_namelen = sizeof(name);
    msg.msg_iov = &iovec;
    msg.msg_iovlen = 1;
    msg.msg_control = (caddr_t)CMSG_ALIGN( (unsigned long)ctrlBuffer );
    msg.msg_controllen = sizeof( ctrlBuffer ) -
        ( (char*)msg.msg_control - (char*)ctrlBuffer );
    msg.msg_flags = 0;

    int r = recvmsg( sock, &msg, 0 );
    if ( r < 0 ) {
        fprintf( stderr, "recvmsg error %s\n", strerror( errno ) );
        return -1;
    }

    if ( msg.msg_flags != 0 ) {
        fprintf( stderr, "recvmsg flags error %d\n", msg.msg_flags );
        return -1;
    }

    in_addr srcAddr = ((sockaddr_in*)msg.msg_name)->sin_addr;
    if ( msg.msg_namelen > 0 ) {
        fprintf( stderr, "recvmsg source %s\n", inet_ntoa( srcAddr ) );
    }

    if ( pktInfo != NULL ) {
        pktInfo->src= srcAddr;
        if ( msg.msg_controllen >= sizeof( struct cmsghdr ) ) {
            struct cmsghdr* cmsg = CMSG_FIRSTHDR( &msg );
            while ( cmsg && cmsg->cmsg_len >= sizeof( struct cmsghdr ) ) {
                switch ( cmsg->cmsg_level ) {
                case IPPROTO_IP: 
                    if ( cmsg->cmsg_type == IP_PKTINFO ) {
                        struct in_pktinfo* info =
                            (struct in_pktinfo*)CMSG_DATA( cmsg );
                        pktInfo->local = info->ipi_spec_dst;
                        pktInfo->dst = info->ipi_addr;
                        pktInfo->ifIndex = info->ipi_ifindex;
                    }
                    break;
                default:
                    fprintf( stderr, "Skip cmsg %d\n", cmsg->cmsg_level );
                    break;
                }
                cmsg = CMSG_NXTHDR( &msg, cmsg );
            }
        }
    }

    return r;
}

int
sendPkt( int sock, const char* buf, uint bufSz, const IpPktInfo* pktInfo )
{
    struct iphdr ipHdr;

    memset( &ipHdr, 0, sizeof(ipHdr) );
    ipHdr.ihl = sizeof(ipHdr) >> 2; // no options
    ipHdr.version = 4;
    ipHdr.tos = IPTOS_PREC_INTERNETCONTROL;
    ipHdr.tot_len = htons( sizeof(ipHdr) + bufSz );
    ipHdr.ttl = pktInfo->ttl;
    ipHdr.protocol = pktInfo->proto;
    ipHdr.saddr = pktInfo->src.s_addr;
    ipHdr.daddr = pktInfo->dst.s_addr;

    struct msghdr msg;
    struct iovec  iovs[2];

    sockaddr_in addr = Util::inetSockAddr( pktInfo->src );

    memset( &msg, 0, sizeof(msg) );
    msg.msg_name = &addr;
    msg.msg_namelen = sizeof(addr);
    msg.msg_iov = iovs;
    msg.msg_iovlen = 2;
    iovs[0].iov_base = (char*)&ipHdr;
    iovs[0].iov_len = sizeof(ipHdr);
    iovs[1].iov_base = (char*)buf;
    iovs[1].iov_len = bufSz;

    int r = sendmsg( sock, &msg, 0 );
    if ( r < 0 ) {
        fprintf( stderr, "sendmsg error %s\n", strerror( errno ) );
        return -1;
    }
    return 0;
}

