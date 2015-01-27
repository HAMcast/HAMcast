// Internal version: Please do not publish!
// (... until released under FreeBSD-like license *g*)
// Code: Sebastian Mies <mies@tm.uka.de>

#ifndef ASIO_IO_SERVICE_H_
#define ASIO_IO_SERVICE_H_

#include<boost/thread.hpp>
#include<boost/asio.hpp>

namespace ariba {
namespace transport {
namespace detail {

/**
 * TODO: Doc
 *
 * @author Sebastian Mies <mies@tm.uka.de>
 */
class asio_io_service {
private:
	int references;
	volatile bool running;
	volatile bool destroy;
	boost::thread* thread;
	boost::asio::io_service* service;

	static asio_io_service* singleton;

	friend class boost::thread;
	friend class boost::detail::thread_data<boost::reference_wrapper<asio_io_service> >;

protected:
	void operator ()();
	asio_io_service();
	~asio_io_service();
	void internal_start();
	void internal_stop();
public:
	static boost::asio::io_service& alloc();
	static void free();
	static void start();
	static void stop();
};

}}} // namespace ariba::transport::detail

#endif /* ASIO_IO_SERVICE_H_ */
