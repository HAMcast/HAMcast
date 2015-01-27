# sources ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

if OMNET
lib_LTLIBRARIES = libtestdht.la
else
bin_PROGRAMS 	= testdht
endif

# compiler flags ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

AM_CPPFLAGS     = -DLINUX -D_LINUX -I../../source -D_REENTRANT
AM_CPPFLAGS    += $(BOOST_CPPFLAGS)

if DEBUG
AM_CPPFLAGS    += -ggdb -DDEBUG -D_DEBUG -O0
endif

if PROFILING
AM_CPPFLAGS    += -pg
endif

if OMNET
AM_CPPFLAGS    += -fPIC -DUNDERLAY_OMNET
endif

# linker flags ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

AM_LDFLAGS      = -L../../source/ariba -lariba

if PROFILING
AM_LDFLAGS     += -pg
endif

if OMNET
AM_LDFLAGS     += -shared -rdynamic
endif

# sources ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

if OMNET

libtestdht_la_SOURCES = \
  DHTTest.cpp \
  DHTTest.h \
  PingPongMessage.cpp \
  PingPongMessage.h

else

#needed to fix autotools bug
testdht_CPPFLAGS = $(AM_CPPFLAGS)

testdht_SOURCES = \
  PingPongMessage.cpp \
  PingPongMessage.h \
  DHTTest.cpp \
  DHTTest.h \
  main.cpp

endif
