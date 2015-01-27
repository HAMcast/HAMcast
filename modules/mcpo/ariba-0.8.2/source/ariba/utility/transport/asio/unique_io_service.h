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


#ifndef UNIQUE_IO_SERVICE_H_
#define UNIQUE_IO_SERVICE_H_

#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>

namespace ariba {
namespace transport {
namespace detail {

/**
 * Multiplexes an asio_io_service so it can be used by multiple components.
 * This class is _not_ threadsafe (use only from ariba thread/system queue)
 *
 * @author Mario Hock <mario@omnifile.org>, Michael Tänzer <neo@nhng.de>
 */
class unique_io_service : public boost::noncopyable
{
public:
    unique_io_service();
    ~unique_io_service();
    
    /**
     * Returned io_service is only valid during the lifetime of the
     * unique_io_service object. 
     */
    boost::asio::io_service& get_asio_io_service();
    
    /**
     * Executes the io_service::run() method in a parallel thread.
     * Multiple calls don't lead to multiple threads.
     */
    void start();
    
    /**
     * Make the io_service::run() method return and stop thread if this is the
     * last stop() call (number of start() calls == number of stop() calls)
     */
    void stop();

private:
    static void thread_function();
    
    
private:
    static boost::asio::io_service* asio_io_service;
    static int asio_io_service_ref_count;
    
    static boost::thread* thread;
    static int run_count;
    
    static boost::mutex thread_stopped_mutex;
    static boost::condition_variable thread_stopped_cond;
    static bool thread_stopped;
    
    bool running;
};

}}} // namespace ariba::transport::detail

#endif /* UNIQUE_IO_SERVICE_H_ */
