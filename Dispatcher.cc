#include <assert.h>
#include <sys/select.h>
#include <unistd.h>
#include <stdio.h>

#include "Dispatcher.h"

using namespace Dispatcher;

static MainLoop* mainLoop_ = 0;

struct MainLoop::Impl {
    
    TimerQueue* timerQueue_;
    typedef std::map< int, IOHandler* > IOHandlerColl;

    IOHandlerColl readHandlers_;
    IOHandlerColl writeHandlers_;
    IOHandlerColl errorHandlers_; // unused

    int run() {
        fd_set rfds, wfds, efds;
   
        while (1) {
            FD_ZERO( &rfds );
            FD_ZERO( &wfds );
            FD_ZERO( &efds );
            int maxSock = 0;
            
            for ( IOHandlerColl::const_iterator it = readHandlers_.begin();
                  it != readHandlers_.end(); it++ ) {
                maxSock = maxSock > it->first ? maxSock : it->first;
                FD_SET( it->first, &rfds );
            }
            for ( IOHandlerColl::const_iterator it = writeHandlers_.begin();
                  it != writeHandlers_.end(); it++ ) {
                maxSock = maxSock > it->first ? maxSock : it->first;
                FD_SET( it->first, &wfds );
            }
            long long timeoutMs = timerQueue_->minFireTime();
            timeval timeout = { timeoutMs / 1000, ( timeoutMs % 1000 ) * 1000 };
            int r = select( maxSock + 1, &rfds, &wfds, &efds, &timeout );
            if ( r > 0 ) {
                checkHandlers( rfds, IORead, readHandlers_ );
                checkTimerQueue();
                checkHandlers( wfds, IOWrite, writeHandlers_ );
                checkTimerQueue();                
            } else {
                checkTimerQueue();
            }
        }
        return 0;
    }

    void checkHandlers( const fd_set& fds, IOFlag flag,
                        IOHandlerColl& handlers ) {
        for ( IOHandlerColl::iterator it = handlers.begin();
              it != handlers.end(); it++ ) {
            if ( FD_ISSET( it->first, &fds ) ) {
                if ( flag == IORead ) {
                    it->second->handleSocket( it->first, IORead );
                } else if ( flag == IOWrite ) {
                    it->second->handleSocket( it->first, IOWrite );
                }
            }
        }
    }

    void checkTimerQueue() {
        timerQueue_->fireTimers();
    }

    int addIOHandler( int sock, uint flags, IOHandler* handler ) {
        if ( flags & IORead ) {
            if ( readHandlers_.find( sock ) != readHandlers_.end() ) {
                fprintf( stderr, "IORead exists for %d\n", sock );
                return -1;
            }
            readHandlers_[ sock ] = handler;
        }
        if ( flags & IOWrite ) {
            if ( writeHandlers_.find( sock ) != writeHandlers_.end() ) {
                fprintf( stderr, "IOWrite exists for %d\n", sock );
                return -1;
            }
            writeHandlers_[ sock ] = handler;
        }
        return 0;
    }

    int removeIOHandler( int sock, uint flags, IOHandler* handler ) {
        if ( flags & IORead ) {
            IOHandlerColl::iterator it = readHandlers_.find( sock );
            if ( it != readHandlers_.end() ) {
                readHandlers_.erase( it );
            }
        }
        if ( flags & IOWrite ) {
            IOHandlerColl::iterator it = writeHandlers_.find( sock );
            if ( it != writeHandlers_.end() ) {
                writeHandlers_.erase( it );
            }
        }
        return 0;
    }

};

MainLoop::MainLoop( TimerQueue* queue )
{
    impl_ = new Impl;
    impl_->timerQueue_ = queue;
}

MainLoop::~MainLoop()
{
    delete impl_;
}

int
MainLoop::run()
{
    return impl_->run();
}

MainLoop*
MainLoop::init( TimerQueue* timerQueue )
{
    if ( mainLoop_ != 0 ) {
        assert( mainLoop_->impl_->timerQueue_ == timerQueue );
        return mainLoop_;
    }
    assert( timerQueue != 0 );
    mainLoop_ = new MainLoop( timerQueue );
    return mainLoop_;
}

int
MainLoop::addIOHandler( int sock, uint flags, IOHandler* handler )
{
    return impl_->addIOHandler( sock, flags, handler );
}

int
MainLoop::removeIOHandler( int sock, uint flags, IOHandler* handler )
{
    return impl_->removeIOHandler( sock, flags, handler );
}

