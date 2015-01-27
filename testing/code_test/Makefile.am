###############################################################################
#                          HAMcast program template                           #
###############################################################################

ACLOCAL_AMFLAGS = -I m4

## replace foo with your program name
noinst_PROGRAMS = test

## replace "foo" with your program name and "foo.cpp" with your source files
test_SOURCES = src/test.cpp

AM_CXXFLAGS = $(BOOST_CPPFLAGS) $(HAMCAST_CPPFLAGS)
AM_LDFLAGS = $(BOOST_LDFLAGS) $(HAMCAST_LDFLAGS)

## DELETE all lines you don't need
## important: hamcast requires:
##            - BOOST_ASIO_LIB
##            - BOOST_REGEX_LIB
##            - BOOST_THREAD_LIB
##            - BOOST_DATE_TIME_LIB
##            - BOOST_FILESYSTEM_LIB
LIBS = \
	$(BOOST_ASIO_LIB) \
	$(BOOST_WAVE_LIB) \
	$(BOOST_REGEX_LIB) \
	$(BOOST_PYTHON_LIB) \
	$(BOOST_SYSTEM_LIB) \
	$(BOOST_THREAD_LIB) \
	$(BOOST_SIGNALS_LIB) \
	$(BOOST_DATE_TIME_LIB) \
	$(BOOST_IOSTREAMS_LIB) \
	$(BOOST_FILESYSTEM_LIB) \
	$(BOOST_SERIALIZATION_LIB) \
	$(BOOST_WSERIALIZATION_LIB) \
	$(BOOST_PROGRAM_OPTIONS_LIB) \
	$(BOOST_TEST_EXEC_MONITOR_LIB) \
	$(BOOST_UNIT_TEST_FRAMEWORK_LIB) \
	$(HAMCAST_LIB)

