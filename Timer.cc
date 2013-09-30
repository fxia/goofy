#include <assert.h>
#include <sys/time.h>
#include "Timer.h"

long long
Util::getCurrentTimeInMs()
{
    struct timeval currentTime;
    gettimeofday( &currentTime, 0 );
    return currentTime.tv_sec * 1000 + currentTime.tv_usec / 1000;
}

static const uint Timer_Granularity = 100;

struct Timer {
    
    uint         token_;
    long long    fireTime_;
    uint         interval_;
    bool         cancelled_;
    TimerFunc*   func_;
    void*        data_;
    
    Timer( uint token, TimerFunc func, uint offset, uint interval, void* data )
        : token_(token), interval_(interval), cancelled_(false),
          func_(func), data_(data) {
        assert( interval == 0 || interval > Timer_Granularity );
        // Add jitter here ?
        fireTime_ = Util::getCurrentTimeInMs() + offset;
    }

    int fire() {
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

   bool operator < ( const Timer& timer ) const {
       return fireTime_ < timer.fireTime_;
   }

};

struct TimerQueue::Impl {
    
    typedef std::priority_queue< Timer* > TimerColl;
    TimerColl timerQueue_;

    typedef std::map< uint, Timer* > TimerIndex;
    TimerIndex timerIndex_;

    uint nextToken_;
    uint cancelCount_;

    Impl() : nextToken_(1), cancelCount_(0) {}

    ~Impl() {
        for ( TimerIndex::iterator it = timerIndex_.begin();
              it != timerIndex_.end(); it++ ) {
            delete it->second;
        }
    }
    
    int addTimer( TimerFunc func, uint offset, uint interval, void* data ) {
        uint token = nextToken_++;
        Timer* timer = new Timer( token, func, offset, interval, data );
        timerQueue_.push( timer );
        timerIndex_[token] = timer;
        return token;
    }

    void cancelTimer( uint token) {
        TimerIndex::iterator it = timerIndex_.find( token );
        if ( it == timerIndex_.end() ) {
            return;
        }
        it->second->fireTime_ = -1;
        it->second->cancelled_ = true;
        cancelCount_++;
    }

    long long minFireTime() {
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

    /* Delete front of the priority queue */
    void deleteFrontTimer() {
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

    int fireTimers() {
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
};

TimerQueue::TimerQueue()
{
    impl_ = new Impl();
}

TimerQueue::~TimerQueue()
{
    delete impl_;
}

int
TimerQueue::addTimer( TimerFunc func, uint offset, uint interval, void* data )
{
    return impl_->addTimer( func, offset, interval, data );
}

void
TimerQueue::cancelTimer( uint token )
{
    impl_->cancelTimer( token );
}

long long
TimerQueue::minFireTime()
{
    return impl_->minFireTime();
}

int
TimerQueue::fireTimers()
{
    return impl_->fireTimers();
}

uint
TimerQueue::queueSize( bool countCancel )
{
    return ( countCancel
             ? impl_->timerQueue_.size()
             : impl_->timerQueue_.size()  - impl_->cancelCount_ );
}

