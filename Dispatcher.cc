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
    IOHandlerColl errorHandlers_;

    void run() {
        fd_set rfds, wfds, efds;
   
        while (1) {
            FD_ZERO( &rfds );
            FD_ZERO( &wfds );
            FD_ZERO( &efds );
            int maxSock = 0;
            
            for ( IOHandlerColl::const_iterator it = readHandlers_.begin();
                  it != readHandlers_.end(); it++ ) {
                maxSock = max( maxSock, it->second->socket() );
                FD_SET( it->second->socket(), &rfds );
            }
            for ( IOHandlerColl::const_iterator it = writeHandlers_.begin();
                  it != writeHandlers_.end(); it++ ) {
                maxSock = max( maxSock, it->second->socket() );
                FD_SET( it->second->socket(), &wfds );
            }
            long long timeoutMs = timerQueue_->minFireTime();
            timeval timeout = { timeoutMs / 1000, ( timeoutMs % 1000 ) * 1000 };
            r = select( maxSock + 1, &rfds, &wfds, &efds, &timeout );
            if ( r > 0 ) {
                checkReadHandlers( &rfds );
                checkTimerQueue();
                checkWriteHandlers( &wfds );
                checkTimerQueue();                
            } else {
                checkTimerQueue();
            }
        }
    }

    void checkReadHandlers( const fd_set& fds ) {
        for ( IOHandlerColl::iterator it = readHandlers_.begin();
              it != readHandlers_.end(); it++ ) {
            if ( FD_ISSET( it->first, &fds ) ) {
                it->second->handleRead();
            }
        }
    }

    void checkWriteHandlers( const fd_set& fds ) {
        for ( IOHandlerColl::iterator it = writeHandlers_.begin();
              it != writeHandlers_.end(); it++ ) {
            if ( FD_ISSET( it->first, &fds ) ) {
                it->second->handleWrite();
            }
        }
    }

    void checkTimerQueue() {
        timerQueue_->fireTimers();
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

MainLoop::run()
{
    impl->run();

}

MainLoop*
MainLoop::init( Timer::Queue* timerQueue )
{
    if ( mainLoop_ != 0 ) {
        assert( mainLoop_->impl_->timerQueue_ == timerQueue );
        return mainLoop_;
    }
    assert( timerQueue != 0 );
    mainLoop_ = new MainLoop( timerQueue );
}

int
MainLoop::addIOHandler( int sock, uint flags, IOHandler* handler )
{
    if ( flags & IORead ) {
        if ( impl_->readHandlers_.find( sock ) != impl_->readHandlers_.end() ) {
            fprintf( stderr, "IORead exists for %d\n", sock );
            return -1;
        }
        impl_->readHandlers_[ sock ] = handler;
    }
    if ( flags & IOWrite ) {
        if ( impl_->writeHandlers_.find( sock ) != impl_->writeHandlers_.end() ) {
            fprintf( stderr, "IOWrite exists for %d\n", sock );
            return -1;
        }
        impl_->writeHandlers_[ sock ] = handler;
    }
    return 0;
}

int
MainLoop::removeIOHandler( int sock, uint flags, IOHandler* handler )
{
    if ( flags & IORead ) {
        Impl::IOHandlerColl::iterator it = impl_->readHandlers_.find( sock );
        if ( it != impl_->readHandlers_.end() ) {
            impl_->readHandlers_.erase( it );
        }
    }
    if ( flags & IOWrite ) {
        Impl::IOHandlerColl::iterator it = impl_->writeHandlers_.find( sock );
        if ( it != impl_->writeHandlers_.end() ) {
            impl_->writeHandlers_.erase( it );
        }
    }
    return 0;
}

