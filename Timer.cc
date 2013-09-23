#include <assert.h>
#include <sys/time.h>
#include "Timer.h"

uint Timer::cancelCount_ = 0;
Timer::TimerQueue Timer::timerQueue_;
Timer::TimerIndex Timer::timerIndex_;

static uint nextToken = 1;

#define Timer_Granularity 100

long long
Util::getCurrentTimeInMs()
{
    struct timeval currentTime;
    gettimeofday( &currentTime, 0 );
    return currentTime.tv_sec * 1000 + currentTime.tv_usec / 1000;
}

int
Timer::addTimer( TimerFunc func, uint offset, uint interval, void* data )
{
    uint token = nextToken++;
    Timer* timer = new Timer( token, func, offset, interval, data );
    timerQueue_.push( timer );
    timerIndex_[token] = timer;
    return token;
}

void
Timer::cancelTimer( uint token )
{
    TimerIndex::iterator it = timerIndex_.find( token );
    if ( it == timerIndex_.end() ) {
        return;
    }
    it->second->cancelled_ = true;
    cancelCount_++;
}

Timer::Timer( uint token, TimerFunc* func, uint offset, uint interval,
              void* data )
    : token_(token), interval_(interval), cancelled_(false),
      func_(func), data_(data)
{
    assert( interval == 0 || interval > Timer_Granularity );
    // Add jitter here ?
    fireTime_ = Util::getCurrentTimeInMs() + offset;
}

Timer::~Timer()
{
}

long long
Timer::minFireTime()
{
    Timer* timer;
    while ( timerQueue_.size() > 0 ) {
        timer = timerQueue_.top();
        if ( timer->cancelled_ ) {
            deleteFrontTimer();
        } else {
            return timer->fireTime_;
        }
    }
    return -1;
}

void
Timer::deleteFrontTimer()
{
    Timer* timer = timerQueue_.top();
    TimerIndex::iterator it = timerIndex_.find( timer->token_ );
    assert( it != timerIndex_.end() );
    if ( timer->cancelled_ ) {
        cancelCount_--;
    }
    timerQueue_.pop();
    timerIndex_.erase( it );
    delete timer;
}

int
Timer::fireTimers()
{
    long long currentTime = Util::getCurrentTimeInMs();

    Timer* timer;
    while ( timerQueue_.size() > 0 ) {
        timer = timerQueue_.top();
        if ( timer->cancelled_ ) {
            deleteFrontTimer();
        } else {
            if ( timer->fireTime_ - currentTime > Timer_Granularity ) {
                break;
            }
            timer->fire();
            if ( timer->fireTime_ < 0 || timer->cancelled_ ) {
                deleteFrontTimer();
            } else {
                // Re-insert this timer
                timerQueue_.pop();
                timerQueue_.push( timer );
                timerIndex_[ timer->token_ ] = timer;
            }
        }
    }
    return 0;
}

uint
Timer::queueSize( bool countCancel )
{
    return ( countCancel ? timerQueue_.size()
             : timerQueue_.size()  - cancelCount_ );
}

int
Timer::fire()
{
    ( func_ )( token_, data_ );
    if ( interval_ == 0 ) {
        fireTime_ = -1; // one-shot
    } else if ( cancelled_ ) {
        fireTime_ = -1; 
    } else {
        fireTime_ = Util::getCurrentTimeInMs() + interval_;
    }
    return 0;
}
