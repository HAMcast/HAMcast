// [License]
// The Ariba-Underlay Copyright
//
// Copyright (c) 2008-2012, Institute of Telematics, Universität Karlsruhe (TH)
//
// Institute of Telematics
// Universität Karlsruhe (TH)
// Zirkel 2, 76128 Karlsruhe
// Germany
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE INSTITUTE OF TELEMATICS ``AS IS'' AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE ARIBA PROJECT OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// The views and conclusions contained in the software and documentation
// are those of the authors and should not be interpreted as representing
// official policies, either expressed or implied, of the Institute of
// Telematics.
// [License]

/**
 * @author Mario Hock <mario@omnifile.org>, Michael Tänzer <neo@nhng.de>
 */

#include "unique_io_service.h"

#include <boost/asio.hpp>
#include <boost/thread.hpp>

namespace ariba {
namespace transport {
namespace detail {

using namespace std;


/* init static members */
boost::asio::io_service* unique_io_service::asio_io_service = NULL;
int unique_io_service::asio_io_service_ref_count = 0;
boost::thread* unique_io_service::thread = NULL;
int unique_io_service::run_count = 0;
boost::mutex unique_io_service::thread_stopped_mutex;
boost::condition_variable unique_io_service::thread_stopped_cond;
bool unique_io_service::thread_stopped = true;


unique_io_service::unique_io_service():
        running(false)
{
    if ( asio_io_service_ref_count == 0 )
    {
        asio_io_service = new boost::asio::io_service();
    }
    
    asio_io_service_ref_count++;
}



unique_io_service::~unique_io_service()
{
    assert( !running );
    
    asio_io_service_ref_count--;
    
    if ( asio_io_service_ref_count <= 0 )
    {
        delete asio_io_service;
        asio_io_service = NULL;
    }
}



boost::asio::io_service & unique_io_service::get_asio_io_service()
{
    assert(asio_io_service != NULL );
    
    return *asio_io_service;
}



void unique_io_service::start()
{
    assert(!running);
    
    boost::mutex::scoped_lock lock(thread_stopped_mutex);
    if (thread_stopped)
    {
    	thread_stopped = false;
        thread = new boost::thread(&unique_io_service::thread_function);
    }
    
    run_count++;
    running = true;
}



void unique_io_service::stop()
{
    assert( running );
    
    running = false;
    run_count--;
    
    if ( run_count == 0 )
    {
        asio_io_service->stop();
        
        boost::mutex::scoped_lock lock(thread_stopped_mutex);
        while (!thread_stopped)
        {
            thread_stopped_cond.wait(lock);
        }
        delete thread;
    }
}


void unique_io_service::thread_function()
{
    asio_io_service->run();
    
    {
        boost::mutex::scoped_lock lock(thread_stopped_mutex);
        thread_stopped = true;
    }
    thread_stopped_cond.notify_one();
}


}}} // namespace ariba::transport::detail
