#ifndef __goofy_dispatcher_h__
#define __goofy_dispatcher_h__

#include "Timer.h"

namespace Dispatcher {

enum IOFlag {
    IORead      = 0x01,
    IOWrite     = 0x02,
};
    
struct IOHandler {
    virtual int handleSocket( int sock, IOFlag flag ) = 0;
};

struct MainLoop {

    static MainLoop* init( TimerQueue* timerQueue );

    int addIOHandler( int sock, uint flags, IOHandler* handler );

    int removeIOHandler( int sock, uint flags, IOHandler* handler );
    
    int run();

private:
    MainLoop( TimerQueue* timerQueue );
    
    struct Impl;
    Impl*  impl_;
    
public:
    ~MainLoop();
};

};
    
#endif

