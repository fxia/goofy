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
   uint token = Timer::addTimer( timer1, 0, 2000, (void*)"abcd" );

   int count = 0;
   long long currentTime, fireTime;
   while ( count < 3 ) {
      sleep(3);
      currentTime = Util::getCurrentTimeInMs();
      fireTime = Timer::minFireTime();
      fprintf( stderr, "Before fire timer %lld\n", currentTime - fireTime );
      fprintf( stderr, "Queue size %d\n", Timer::queueSize() );
      Timer::fireTimers();
      count++;
   }

   Timer::cancelTimer( token );
   assert( Timer::queueSize() == 0 );
   assert( Timer::queueSize( true ) == 1 );
   return 0;
}
