# sources ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

if OMNET
lib_LTLIBRARIES = libmcpopingpong.la
else
bin_PROGRAMS 	= mcpopingpong
endif

# compiler flags ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

AM_CPPFLAGS     = -Wall -DLINUX -D_LINUX -I../../source -D_REENTRANT
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

AM_LDFLAGS      = -L../../source/mcpo -lmcpo

if PROFILING
AM_LDFLAGS     += -pg
endif

if OMNET
AM_LDFLAGS     += -shared -rdynamic
endif

# sources ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

if OMNET

libmcpopingpong_la_SOURCES = \
  PingPong.cpp \
  PingPong.h \
  PingPongMessage.cpp \
  PingPongMessage.h
else

#needed to fix autotools bug
mcpopingpong_CPPFLAGS = $(AM_CPPFLAGS)

mcpopingpong_SOURCES = \
  PingPong.cpp \
  PingPong.h \
  PingPongMessage.cpp \
  PingPongMessage.h \
  main.cpp

endif
