#ifndef __goofy_dispatcher_h__
#define __goofy_dispatcher_h__

#include "Timer.h"

namespace Dispatcher {

enum {
    IORead      = 0x01,
    IOWrite     = 0x02,
};
    
struct IOHandler {
    virtual int handleSocket( int sock, uint flags ) = 0;
};

struct MainLoop {

    static MainLoop* init( Timer::Queue* timerQueue );

    int addIOHandler( int sock, uint flags, IOHandler* handler );

    int removeIOHandler( int sock, uint flags, IOHandler* handler );
    
    int run();

private:
    MainLoop( Timer::Queue* timerQueue );
    
    struct Impl;
    Impl*  impl_;
    
public:
    ~MainLoop();
};

};
    
#endif

