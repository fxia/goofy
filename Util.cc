#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>

#include "Util.h"

#define MAX_NUM_BUFFERS 30
#define STR_BUFFER_SIZE 1024

char*
Util::getStrBuffer( size_t* size )
{
    static char* buffers[ MAX_NUM_BUFFERS ];
    static uchar initialized = 0;
    static int buffer_index = 0;

    if ( !initialized ) {
        memset( buffers, 0, sizeof( buffers ) );
        initialized = 1;
    }
   
    buffer_index++;
    if ( buffer_index == MAX_NUM_BUFFERS ) {
        buffer_index = 0;
    }
    if ( buffers[ buffer_index ] == 0 ) {
        buffers[ buffer_index ] = (char*)malloc( STR_BUFFER_SIZE );
    }
    *size = STR_BUFFER_SIZE;
    memset( buffers[ buffer_index ], 0, STR_BUFFER_SIZE );
    return buffers[ buffer_index ];
}

const char*
Util::ipAddrString( uint ipAddr )
{
    struct in_addr in = { ipAddr };

    size_t sz;
    char* buf = getStrBuffer( &sz );
    snprintf( buf, sz, "%s", inet_ntoa( in ) );
    return buf;
}

ushort
Util::inetChecksum( const char* bytes, uint nbytes )
{
    /* Copied from quagga */
    ushort           *ptr = (ushort*)bytes;
    register uint    sum;
    ushort           oddbyte;
    register ushort  answer;	

    sum = 0;
    while (nbytes > 1)  {
        sum += *ptr++;
        nbytes -= 2;
    }
   
    if (nbytes == 1) {
        oddbyte = 0;
        *((u_char *) &oddbyte) = *(u_char *)ptr;
        sum += oddbyte;
    }
    sum  = (sum >> 16) + (sum & 0xffff);	
    sum += (sum >> 16);
    answer = ~sum;
    return answer;
}

int
Util::getInterfaceAddrMask( const char* ifName, uint* addr, uint* mask )
{
    struct ifreq req;
    memset( &req, 0, sizeof(req) );
    snprintf( req.ifr_name, IFNAMSIZ, "%s", ifName );

    int fd = socket( AF_INET, SOCK_DGRAM, 0 );
    if ( fd < 0 ) {
        fprintf( stderr, "Cannot open socket %s\n", strerror( errno ) );
        return -1;
    }
   
    if ( ioctl( fd, SIOCGIFADDR, &req ) < 0 ) {
        fprintf( stderr, "ioctl error %s\n", strerror( errno ) );
        return -1;
    }

    in_addr inetAddr = ((sockaddr_in*)(&req.ifr_ifru.ifru_addr))->sin_addr; 

    memset( &req.ifr_ifru, 0, sizeof(req.ifr_ifru) );
    if ( ioctl( fd, SIOCGIFNETMASK, &req ) < 0 ) {
        fprintf( stderr, "ioctl error %s\n", strerror( errno ) );
        return -1;
    }

    in_addr inetMask = ((sockaddr_in*)(&req.ifr_ifru.ifru_addr))->sin_addr;

    fprintf( stderr, "%s: %s, %s\n", ifName, inet_ntoa(inetAddr),
             inet_ntoa(inetMask) );

    *addr = inetAddr.s_addr;
    *mask = inetAddr.s_addr;

    return 0;
}

   
in_addr
Util::v4MaskLen2Ip( uint len )
{
    assert( len < 33 );
    in_addr addr = { len == 0 ? 0 : htonl( 0xFFFFFFFF << ( 32 - len ) ) };
    return addr;
}

uint
Util::v4MaskIp2Len( in_addr mask )
{
    return mask.s_addr == 0 ? 32 : 32 - __builtin_clz( mask.s_addr );
}

in_addr
Util::v4PrefixIp( in_addr ipAddr, uint prefixLen )
{
    assert( prefixLen < 33 );
    in_addr maskAddr = v4MaskLen2Ip( prefixLen );
    in_addr addr = { ipAddr.s_addr & maskAddr.s_addr };
    return addr;
}

