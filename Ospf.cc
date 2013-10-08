#include "Ospf.h"

using namespace Ospf;

/*----------------------------------------------------------------------------
 * Packet
 *
 *--------------------------------------------------------------------------*/

static int checkPacketHeader( const PacketHeader* hdr );

Packet::Packet( const char* buf, uint len, bool copy,
                const IpPkt::PktInfo& info )
    : pktLen( len ), info_( info ), isCopy_( copy )
{
    if ( copy ) {
        pkt_ = new char[ len ];
        memcpy( pkt_, buf, len );
    } else {
        pkt_ = buf;
    }
}

const Packet* Packet::makePacket( const char* buf, uint len, bool copy,
                                  IpPkt::PktInfo& info )
{
    if ( len < sizeof( PacketHeader ) ) {
        return 0;
    }

    const PacketHeader* hdr = (const PacketHeader*)buf;
    if ( checkPacketHeader( hdr ) < 0 ) {
        return 0;
    }
    Packet* pkt = new Packet( buf, len, copy, info );
}

int checkPacketHeader( const PacketHeader* hdr )
{
    if ( hdr->version != 2 ) {
        return -1;
    }
    if ( hdr->type > PacketType::LSAck ) {
        return -1;
    }
    return 0;
}

/*--------------------------------------------------------------------------
 * Instance
 *
 *--------------------------------------------------------------------------*/
struct Instance::Impl {

    Interface* findInterface( const IpPkt::PktInfo& info ) {
        
    }
    
    int verifyPacketHeader( const Packet* packet ) {
        
    }
    
    int verifyHelloPacket( const Packet* hello ) {
        
    }
};

 
