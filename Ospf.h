#ifndef __goofy_ospf_h__
#define __goofy_packet_h__

#include <netinet/in.h>
#include "Packet.h"

namespace Ospf {

struct PacketHeader {
    uchar    version;
    uchar    type;
    ushort   length;
    in_addr  routerId;
    in_addr  areaId;
    ushort   checksum;
    ushort   authType;
    uchar    auth[8];
    uchar    packet[0];
};

struct PacketType {
    enum {
        Hello     = 1,
        DD        = 2,
        LSRequest = 3,
        LSUpdate  = 4,
        LSAck     = 5,
        End
    }
};

struct PacketHello {
    in_addr  networkMask;
    ushort   interval;
    uchar    options;
    uchar    priority;
    uint     deadInterval;
    in_addr  dr;
    in_addr  bdr;
    in_addr  neighbors[0];
};

struct Option {
    enum {
        DC = 0x20,
        EA = 0x10,
        NP = 0x08,
        MC = 0x04,
        E  = 0x02,
    };
};

/* OSPF packet */
struct Packet {
    
    Packet( const char* pkt, uint len, bool copy,
            const IpPkt::PktInfo* info );
    
    ~Packet();

    const PacketHeader* header() const;

    const PacketHello* hello() const;

    uint length() const;

    static const Packet* makePacket( const char* buf, uint len, bool copy,
                                     const IpPkt::PktInfo& info );
    
protected:
    char*       pkt_;
    uint        pktLen_;
    PktInfo     info_;
    bool        isCopy_;
};

inline uint Packet::length() const
{
    return pktLen_;
}

/*--------------------------------------------------------------------------
 * OSPF data structures
 *
 *--------------------------------------------------------------------------*/
 
struct Interface {
    
    enum State {
        Down      = 1,
        Loopback  = 2,
        Waiting   = 3,
        P2P       = 4,
        Backup    = 5,
        DROther   = 6,
        DR        = 7,
        End       = 8
    };

    in_addr ipAddress() const { return ipAddr_; }
    in_addr mask() const { return mask_; }
    in_addr prefix() const;    
    uint ifIndex() const { return ifIndex_; }
    const char* ifName() const { return ifName_; }
    in_addr areaId() const { return areaId_; }
    State state() const { return state_; }
    ushort interval() const { return interval_; }
    ushort deadInterval() const { return deadInterval_; }
    ushort rmxInterval() const { return rmInterval_; }
    in_addr dr() const { return dr_; }
    in_addr bdr() const { return bdr_; }
    
    void setArea( in_addr areaId );
    void setState( State state );
    void setInterval( ushort interval );
    void setDeadInterval( ushort deadInterval );
    void setRmxInterval( ushort rmxInterval );
    void setDR( in_addr dr );
    void setBDR( in_addr bdr );
   
    void addNeighbor( in_addr routerId, in_addr ngbIp );

    uint numNeighborRouters() const { return neighborByRouterId_.size(); }
    uint numNeighborIps( uint routerId ) const;
    
    /* Get peer interface Ips of a neighbor */
    uint getNeighborIps( in_addr routerId, in_addr* ips, uint nips ) const;

    /* Get list of neighbor router ids */
    uint getNeighborRouterIds( in_addr* nbrs, uint nnbrs ) const;

    /* Get neighbor router id of a peer interface ip */
    uint getNeighborRouterId( in_addr neighborIp ) const;
   
    const char* asString() const;

private:
    Interface( in_addr ip, in_addr mask, uint ifIndex, const char* name );
    ~Interface();
   
    in_addr  ipAddr_;
    in_addr  mask_;
    uint     ifIndex_;
    char     ifName_[IF_NAMESIZE];   
    uchar    priority_;
    in_addr  dr_;
    in_addr  bdr_;
    ushort   interval_;
    ushort   deadInterval_;
    ushort   rmxInterval_;
    in_addr  areaId_;
    State    state_;

    // may use hash map later
    typedef std::map< in_addr, in_addr > Neighboripcoll;
    NeighborIpColl neighborByIp_;

    typedef std::map< in_addr, vector< in_addr > > NeighborRouterColl;
    NeighborRouterColl neighborByRouterId_;
    
};

struct Neighbor {
    
    Neighbor( in_addr routerId, in_addr neighborIp );
    ~Neighbor();

    enum State {
        Down              = 1,
        Attempt           = 2,
        Init              = 3,
        HelloReceived     = 4,
        OneWay            = 5,
        TwoWay            = 6,
        TwoWay            = 7,      
        ExStart           = 8,
        Exchange          = 9,
        Loading           = 10,
        Full              = 11,
        End               = 12
    };
   
    in_addr routerId() const { return routerId_; }
    in_addr neighborIp() const { return neighborIp_; }
    State state() const { return state_; }
   
private:

    in_addr  routerId_;
    in_addr  neighborIp_;
    ushort   priority_;
    ushort   options_;
    in_addr  dr_;
    in_addr  bdr_;
    uint     ddSeqno_;
    uint     lastDD_;
    
};

struct Area {
    Area( in_addr areaId );
    ~Area();
   
    in_addr areaId() const;

    int addInterface( in_addr interfaceIp );

    int addInterface( Interface* interface );
    void removeInterface( in_addr interfaceIp );
    void removeInterface( Interface* interface );
    int addNeighbor( in_addr routerId, in_addr neighborIp );
    void removeNeighbor( in_addr routerId );

    /* IP of interfaces in the area */
    typedef std::set< in_addr > InterfaceIds;
   
    const InterfaceIds* interfaces() const;

    bool hasInterface( uint ipAddress ) const;

    /* Get neighbor of an interface, if any */
    const Neighbor* getNeighbor( in_addr interfaceIp );
   
private:
    struct Impl;

    in_addr  areaId_;
    Impl*    impl_;
    
};

struct Instance {
    
    Instance( in_addr routerId );
    Instance( const char* routerId );
    ~Instance();

    /* Open a raw socket */
    int init();

    in_addr routerId() const { return routerId_; }

    int addAreaNetwork( const char* area, const char* network );
   
    Interface* addInterface( const char* ifName );
   
    int removeInterface( in_addr ip );
    int removeInterface( in_addr prefix, in_addr mask );

    int receivePacket();

    const Interface* getInterface( in_addr interfaceIp ) const;

    static Instance* getInstanceOfInterface( in_addr interfaceIp );

    /* Send a Hello packet over an interface */
    int sendHello( in_addr interfaceIp );
   
    int receivePacket();
    int processPacket( const Packet* pkt );
    int processHelloPacket( const Packet* pkt );
   
private:
    struct Impl;

    in_addr  routerId_;
    Impl*    impl_;
    
};

};

#endif
