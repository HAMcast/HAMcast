# sources ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

if OMNET
lib_LTLIBRARIES = libpingpong.la
else
bin_PROGRAMS 	= pingpong
endif

# compiler flags ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

AM_CPPFLAGS     = -DLINUX -D_LINUX -I../../source -D_REENTRANT -Lboost_system
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

AM_LDFLAGS      = -L../../source/ariba -lariba -lboost_system

if PROFILING
AM_LDFLAGS     += -pg
endif

if OMNET
AM_LDFLAGS     += -shared -rdynamic
endif

# sources ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

if OMNET

libpingpong_la_SOURCES = \
  PingPong.cpp \
  PingPong.h \
  PingPongMessage.cpp \
  PingPongMessage.h

else

#needed to fix autotools bug
pingpong_CPPFLAGS = $(AM_CPPFLAGS)

pingpong_SOURCES = \
  PingPongMessage.cpp \
  PingPongMessage.h \
  PingPong.cpp \
  PingPong.h \
  main.cpp

endif
