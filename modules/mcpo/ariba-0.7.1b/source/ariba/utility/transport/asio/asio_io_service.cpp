// Internal version: Please do not publish!
// (... until released under FreeBSD-like license *g*)
// Code: Sebastian Mies <mies@tm.uka.de>

#include "asio_io_service.h"

#include <boost/asio.hpp>
#include <boost/thread.hpp>

namespace ariba {
namespace transport {
namespace detail {

using namespace boost::asio;
using namespace std;

asio_io_service* asio_io_service::singleton = NULL;

//#define DBG(x) cout << x << endl;
#define DBG(x)

void asio_io_service::operator ()() {
	running = true;
	DBG("io_service started");
	boost::asio::io_service::work work(*service);
	service->run();
	DBG("io_service stopped");
	if (destroy) {
		delete singleton;
		singleton = NULL;
		DBG(cout << "asio io_service singleton destroyed" << endl);
	}
	running = false;
}

asio_io_service::asio_io_service() :
	references(1), running(false), destroy(false), thread(NULL), service(NULL) {
	service = new io_service();
}

asio_io_service::~asio_io_service() {
	if (running) {
		service->stop();
		thread->join();
	}
	if (thread != NULL) delete thread;
	if (service != NULL) delete service;
	thread = NULL;
	service = NULL;
}

void asio_io_service::internal_start() {
	if (!running) {
		if (thread != NULL) delete thread;
		thread = new boost::thread(boost::ref(*this));
	}
}

void asio_io_service::internal_stop() {
	service->stop();
	singleton->running = false;
}

io_service& asio_io_service::alloc() {
	if (singleton != NULL) {
		DBG("new asio io_service reference");
		singleton->references++;
		return *singleton->service;
	} else {
		DBG("creating new asio io_service singleton");
		singleton = new asio_io_service();
		return *singleton->service;
	}
}

void asio_io_service::free() {
	if (singleton != NULL) {
		DBG("decreasing asio io_service reference");
		singleton->references--;
		if (singleton->references == 0) {
			DBG("request asio io_service destruction");
			if (singleton->running == false) {
				delete singleton;
				singleton = NULL;
				DBG("asio io_service singleton destroyed");
			} else {
				singleton->destroy = true;
				singleton->service->stop();
			}
		}
	}
}

void asio_io_service::start() {
	singleton->internal_start();
}

void asio_io_service::stop() {
	singleton->internal_stop();
}


}}} // namespace ariba::transport::detail
