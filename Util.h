#ifndef __goofy_util_h__
#define __goofy_util_h__

#include <stdlib.h>
#include <netinet/in.h>
#include <net/if.h>

typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned short ushort;

namespace Util {

/* Get next string buffer */
char* getStrBuffer( size_t* size );

/* Copy the static buffer inet_ntoa() uses to a str buffer. */
const char* ipAddrString( uint ipAddr );

ushort inetChecksum( const char* bytes, uint nbytes );

inline sockaddr_in inetSockAddr( in_addr ipAddr )
{
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = 0;
    addr.sin_addr.s_addr = ipAddr.s_addr;
    return addr;
}

int getInterfaceAddrMask( const char* ifName, in_addr* addr, in_addr* mask );

in_addr v4MaskLen2Ip( uint len );

uint v4MaskIp2Len( in_addr mask );

in_addr v4PrefixIp( in_addr ipAddr, uint prefixLen );

/* Simple collector of heap pointer */
struct scope_ptr {
   scope_ptr( char* p ) : ptr_(p) {}
   ~scope_ptr() { free(ptr_); }
private:
   void* ptr_;
};

};

#endif

