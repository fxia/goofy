#ifndef __goofy_timer_h__
#define __goofy_timer_h__

#include <queue>
#include <map>
#include "Util.h"

namespace Util {

long long getCurrentTimeInMs();
   
};

typedef void (TimerFunc)( uint token, void* data );

struct TimerQueue {

    TimerQueue();

    ~TimerQueue();
    
    int addTimer( TimerFunc func, uint offset, uint interval, void* data );
   
    void cancelTimer( uint token );

    long long minFireTime();
   
    int fireTimers();

    uint queueSize( bool countCancel=false );
   
private:

    struct Impl;
    Impl* impl_;
};

#endif
