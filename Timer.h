#ifndef __goofy_timer_h__
#define __goofy_timer_h__

#include <queue>
#include <map>
#include "Util.h"

namespace Util {

long long getCurrentTimeInMs();
   
};


struct Timer {

   typedef void (TimerFunc)( uint token, void* data );
   
   static int addTimer( TimerFunc func, uint offset, uint interval,
                        void* data );
   
   static void cancelTimer( uint token );

   static long long minFireTime();
   
   static int fireTimers();

   static uint queueSize( bool countCancel=false );
   
   bool operator < ( const Timer& timer ) const {
       return fireTime_ < timer.fireTime_;
   }

   ~Timer();
   
private:
   Timer( uint token, TimerFunc func, uint offset, uint interval, void* data );

   int fire();
   
   uint         token_;
   long long    fireTime_;
   uint         interval_;
   bool         cancelled_;
   TimerFunc*   func_;
   void*        data_;

   typedef std::priority_queue< Timer* > TimerQueue;
   static TimerQueue timerQueue_;

   typedef std::map< uint, Timer* > TimerIndex;
   static TimerIndex timerIndex_;
   
   static uint cancelCount_;
   
   static void deleteFrontTimer();
};

#endif
