###############################################################################
#                          HAMcast program template                           #
###############################################################################

ACLOCAL_AMFLAGS = -I m4

## replace foo with your program name
noinst_PROGRAMS = test_receiver

## replace "foo" with your program name and "foo.cpp" with your source files
test_receiver_SOURCES = main.cpp

AM_CXXFLAGS = $(BOOST_CPPFLAGS) $(HAMCAST_CPPFLAGS)
AM_LDFLAGS = $(BOOST_LDFLAGS) $(HAMCAST_LDFLAGS)

## DELETE all lines you don't need
LIBS = \
	$(BOOST_ASIO_LIB) \
	$(BOOST_REGEX_LIB) \
	$(BOOST_THREAD_LIB) \
	$(BOOST_DATE_TIME_LIB) \
	$(BOOST_FILESYSTEM_LIB) \
	$(HAMCAST_LIB)

