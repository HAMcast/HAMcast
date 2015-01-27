lib_LTLIBRARIES           = libmcpo.la
libmcpo_la_SOURCES        =
nobase_libmcpo_la_HEADERS =

# compiler flags ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# read the autotools book 11.4 for setting the versioning number
# this is different from normal versioning schemes and important
# to set correctly as the runtime linker chooses the correct lib
# depending on the versioning information here! This is not the
# project version number!!

libmcpo_la_LDFLAGS = -version-info 0:0:0

# compiler flags ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

AM_CPPFLAGS     = -Wall -DLINUX -D_LINUX -I../ -D_REENTRANT
AM_CPPFLAGS    += $(BOOST_CPPFLAGS)

if DEBUG
AM_CPPFLAGS    += -ggdb -DDEBUG
endif

if PROFILING
AM_CPPFLAGS    += -pg
endif

if OMNET
AM_CPPFLAGS    += -fPIC -DUNDERLAY_OMNET
endif

# linker flags ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

AM_LDFLAGS      = $(BOOST_THREAD_LDFLAGS) $(BOOST_SYSTEM_LDFLAGS)
LIBS           += $(BOOST_THREAD_LIBS) $(BOOST_SYSTEM_LIBS)

if PROFILING
AM_LDFLAGS     += -pg
endif

if OMNET
AM_LDFLAGS     += -shared -rdynamic
endif

# sources and subdirs ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

libmcpo_ladir = ${pkgincludedir}

# ------------> root

libmcpo_la_SOURCES += \
  MCPO.cpp \
  MCPOCluster.cpp \
  MCPOPeerInfo.cpp

nobase_libmcpo_la_HEADERS += \
  MCPO.h \
  MCPOCluster.h \
  MCPOPeerInfo.h \
  combination.h \
  config.h

# ------------> messages

libmcpo_la_SOURCES += \
  messages/MCPOMsg.cc \
  messages/MCPOLayerMsg.cc \
  messages/MCPOMemberMessage.cc \
  messages/MCPOHeartbeat.cc \
  messages/MCPOClusterMergeMsg.cc \
  messages/MCPOLeaderHeartbeat.cc \
  messages/MCPOLeaderTransfer.cc

nobase_libmcpo_la_HEADERS += \
  messages/MCPOMsg.h \
  messages/MCPOLayerMsg.h \
  messages/MCPOMemberMessage.h \
  messages/MCPOHeartbeat.h \
  messages/MCPOClusterMergeMsg.h \
  messages/MCPOLeaderHeartbeat.h \
  messages/MCPOLeaderTransfer.h

# ------------> timer

libmcpo_la_SOURCES += \
  timer/MCPOTimer.cpp

nobase_libmcpo_la_HEADERS += \
  timer/MCPOTimer.h
