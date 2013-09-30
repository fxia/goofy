#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "../Timer.h"

static int fireCount = 0;

void timer1( uint token, void* str )
{
    fprintf( stderr, "timer1 %s\n", (const char*)str );
    fireCount++;
}

int main( int argc, char** argv )
{
    TimerQueue timerQueue;
    
    uint token = timerQueue.addTimer( timer1, 0, 2000, (void*)"abcd" );

    int count = 0;
    long long currentTime, fireTime;
    while ( count < 3 ) {
        sleep(3);
        currentTime = Util::getCurrentTimeInMs();
        fireTime = timerQueue.minFireTime();
        fprintf( stderr, "Before fire timer %lld\n", currentTime - fireTime );
        fprintf( stderr, "Queue size %d\n", timerQueue.queueSize() );
        timerQueue.fireTimers();
        count++;
    }

    timerQueue.cancelTimer( token );
    assert( timerQueue.queueSize() == 0 );
    assert( timerQueue.queueSize( true ) == 1 );
    return 0;
}
