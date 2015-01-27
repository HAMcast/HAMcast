# the rocking Ariba library

lib_LTLIBRARIES     = libariba.la

# source files and header files
# header files get installed using
# make install. the nobase_ prefix
# keeps the directory structure

libariba_la_SOURCES        =
nobase_libariba_la_HEADERS =

# compiler flags ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# read the autotools book 11.4 for setting the versioning number
# this is different from normal versioning schemes and important
# to set correctly as the runtime linker chooses the correct lib
# depending on the versioning information here! This is not the
# project version number!!

libariba_la_LDFLAGS = -version-info 0:0:0

# compiler flags ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

AM_CPPFLAGS    = -Wall -DLINUX -D_LINUX -I../ -D_REENTRANT -DSCTP_KERN
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

if LOGCOLORS
AM_CPPFLAGS    += -DLOGCOLORS
endif

# linker flags ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

AM_LDFLAGS      = $(BOOST_THREAD_LDFLAGS) $(BOOST_SYSTEM_LDFLAGS) $(BOOST_REGEX_LDFLAGS)
LIBS           += $(BOOST_THREAD_LIBS) $(BOOST_SYSTEM_LIBS) $(BOOST_REGEX_LIBS)

if PROFILING
AM_LDFLAGS     += -pg
endif

if OMNET
AM_LDFLAGS     += -shared -rdynamic
endif

# sources and subdirs ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# where to put the libs into -> /ariba
# currently we have no version number
# attached to the header/libs folder
libariba_ladir = ${pkgincludedir}

# ------------> tidy interface

libariba_la_SOURCES += \
  AribaModule.cpp \
  CommunicationListener.cpp \
  Identifiers.cpp \
  LinkProperties.cpp \
  DataMessage.cpp \
  Module.cpp \
  Name.cpp \
  Node.cpp \
  NodeListener.cpp \
  SideportListener.cpp \
  SpoVNetProperties.cpp

nobase_libariba_la_HEADERS += \
  ariba.h \
  AribaModule.h \
  CommunicationListener.h \
  Identifiers.h \
  LinkProperties.h \
  Message.h \
  Module.h \
  DataMessage.h \
  Name.h \
  Node.h \
  NodeListener.h \
  SideportListener.h \
  SpoVNetProperties.h \
  config.h
  
# ------------> communication

libariba_la_SOURCES += \
  communication/BaseCommunication.cpp \
  communication/CommunicationEvents.cpp \
  communication/EndpointDescriptor.cpp

nobase_libariba_la_HEADERS += \
  communication/BaseCommunication.h \
  communication/CommunicationEvents.h \
  communication/EndpointDescriptor.h

# ------------> communication :: messages

libariba_la_SOURCES += \
  communication/messages/AribaBaseMsg.cpp

nobase_libariba_la_HEADERS += \
  communication/messages/AribaBaseMsg.h

#------------> communication :: networkinfo

libariba_la_SOURCES += \
  communication/networkinfo/AddressDiscovery.cpp \
  communication/networkinfo/NetworkChangeDetection.cpp \
  communication/networkinfo/NetworkInformation.cpp \
  communication/networkinfo/NetworkInterface.cpp

nobase_libariba_la_HEADERS += \
  communication/networkinfo/AddressDiscovery.h \
  communication/networkinfo/NetworkChangeDetection.h \
  communication/networkinfo/NetworkChangeInterface.h \
  communication/networkinfo/NetworkInterface.h \
  communication/networkinfo/NetworkInformation.h

#------------> overlay

libariba_la_SOURCES += \
  overlay/BaseOverlay.cpp \
  overlay/OverlayBootstrap.cpp \
  overlay/LinkDescriptor.cpp

nobase_libariba_la_HEADERS += \
  overlay/BaseOverlay.h \
  overlay/OverlayBootstrap.h \
  overlay/LinkDescriptor.h

#------------> overlay :: messages

libariba_la_SOURCES += \
  overlay/messages/JoinReply.cpp \
  overlay/messages/JoinRequest.cpp \
  overlay/messages/DHTMessage.cpp \
  overlay/messages/OverlayMsg.cpp

nobase_libariba_la_HEADERS += \
  overlay/messages/JoinReply.h \
  overlay/messages/JoinRequest.h \
  overlay/messages/DHTMessage.h\
  overlay/messages/OverlayMsg.h

#------------> overlay :: modules

libariba_la_SOURCES += \
  overlay/modules/OverlayStructureEvents.cpp \
  overlay/modules/OverlayFactory.cpp \
  overlay/modules/OverlayInterface.cpp

nobase_libariba_la_HEADERS += \
  overlay/modules/OverlayStructureEvents.h \
  overlay/modules/OverlayFactory.h \
  overlay/modules/OverlayInterface.h

#------------> overlay :: modules :: onehop

libariba_la_SOURCES += \
  overlay/modules/onehop/OneHop.cpp \
  overlay/modules/onehop/messages/OneHopMessage.cpp \
  overlay/modules/onehop/messages/NodeListingRequest.cpp \
  overlay/modules/onehop/messages/NodeListingReply.cpp

nobase_libariba_la_HEADERS += \
  overlay/modules/onehop/OneHop.h \
  overlay/modules/onehop/messages/OneHopMessage.h \
  overlay/modules/onehop/messages/NodeListingRequest.h \
  overlay/modules/onehop/messages/NodeListingReply.h

#------------> overlay :: modules :: chord

libariba_la_SOURCES += \
  overlay/modules/chord/Chord.cpp \
  overlay/modules/chord/messages/Discovery.cpp 

nobase_libariba_la_HEADERS += \
  overlay/modules/chord/Chord.h \
  overlay/modules/chord/messages/Discovery.h \
  overlay/modules/chord/detail/chord_routing_table.hpp \
  overlay/modules/chord/detail/comparators.hpp \
  overlay/modules/chord/detail/distances.hpp \
  overlay/modules/chord/detail/minimizer_table.hpp \
  overlay/modules/chord/detail/table_listener.hpp 

#------------> utility

nobase_libariba_la_HEADERS += \
  utility/types.h \
  utility/messages.h \
  utility/serialization.h

#------------> utility :: bootstrap

libariba_la_SOURCES += \
  utility/bootstrap/BootstrapManager.cpp \
  utility/bootstrap/modules/BootstrapModule.cpp \
  utility/bootstrap/modules/multicastdns/MulticastDns.cpp \
  utility/bootstrap/modules/bluetoothsdp/BluetoothSdp.cpp \
  utility/bootstrap/modules/periodicbroadcast/PeriodicBroadcast.cpp \
  utility/bootstrap/modules/periodicbroadcast/PeriodicBroadcastMessage.cpp
  
nobase_libariba_la_HEADERS += \
  utility/bootstrap/BootstrapManager.h \
  utility/bootstrap/BootstrapInformationCallback.h \
  utility/bootstrap/modules/BootstrapModule.h \
  utility/bootstrap/modules/multicastdns/MulticastDns.h \
  utility/bootstrap/modules/bluetoothsdp/BluetoothSdp.h \
  utility/bootstrap/modules/periodicbroadcast/PeriodicBroadcast.h \
  utility/bootstrap/modules/periodicbroadcast/PeriodicBroadcastMessage.h

#------------> utility :: configuration

libariba_la_SOURCES += \
  utility/configuration/ConfigFile.cpp \
  utility/configuration/Configuration.cpp

nobase_libariba_la_HEADERS += \
  utility/configuration/ConfigFile.h \
  utility/configuration/Configuration.h

#------------> utility :: internal

nobase_libariba_la_HEADERS += \
  utility/internal/Utilities.hpp

#------------> utility :: logging

nobase_libariba_la_HEADERS += \
  utility/logging/Logging.h

#------------> utility :: measurement

libariba_la_SOURCES += \
  utility/measurement/PathloadMeasurement.cpp

nobase_libariba_la_HEADERS += \
  utility/measurement/PathloadMeasurement.h

#------------> utility :: addressing

libariba_la_SOURCES += \
  utility/addressing/ip_address.cpp \
  utility/addressing/tcpip_endpoint.cpp \
  utility/addressing/rfcomm_endpoint.cpp \
  utility/addressing/mac_address.cpp \
  utility/addressing/port_address.cpp 

nobase_libariba_la_HEADERS += \
  utility/addressing/ip_address.hpp \
  utility/addressing/test_addressing.hpp \
  utility/addressing/tcpip_endpoint.hpp \
  utility/addressing/endpoint_set.hpp \
  utility/addressing/addressing.hpp \
  utility/addressing/detail/compare_to_operators.hpp \
  utility/addressing/detail/address_convenience.hpp \
  utility/addressing/rfcomm_endpoint.hpp \
  utility/addressing/mac_address.hpp \
  utility/addressing/port_address.hpp \
  utility/addressing/facades/to_string_v.hpp \
  utility/addressing/facades/address_v.hpp \
  utility/addressing/facades/vfacade.hpp \
  utility/addressing/facades/comparable_v.hpp \
  utility/addressing/facades/to_bytes_v.hpp

#------------> utility :: transport

libariba_la_SOURCES += \
  utility/transport/tcpip/tcpip.cpp \
  utility/transport/tcpip/protlib/timer_module.cpp \
  utility/transport/tcpip/protlib/setuid.cpp \
  utility/transport/tcpip/protlib/queuemanager.cpp \
  utility/transport/tcpip/protlib/messages.cpp \
  utility/transport/tcpip/protlib/fqueue.cpp \
  utility/transport/tcpip/protlib/fastqueue.c \
  utility/transport/tcpip/protlib/eclock_gettime.c \
  utility/transport/tcpip/protlib/tp_over_udp.cpp \
  utility/transport/tcpip/protlib/connectionmap_uds.cpp \
  utility/transport/tcpip/protlib/network_message.cpp \
  utility/transport/tcpip/protlib/threadsafe_db.cpp \
  utility/transport/tcpip/protlib/timer.cpp \
  utility/transport/tcpip/protlib/address.cpp \
  utility/transport/tcpip/protlib/connectionmap.cpp \
  utility/transport/tcpip/protlib/tp.cpp \
  utility/transport/tcpip/protlib/tp_over_tcp.cpp \
  utility/transport/tcpip/protlib/configuration.cpp \
  utility/transport/tcpip/protlib/ie.cpp \
  utility/transport/tcpip/protlib/threads.cpp \
  utility/transport/tcpip/protlib/logfile.cpp \
  utility/transport/transport_peer.cpp \
  utility/transport/rfcomm/rfcomm.cpp \
  utility/transport/asio/asio_io_service.cpp

nobase_libariba_la_HEADERS += \
  utility/transport/test_transport.hpp \
  utility/transport/tcpip/tcpip.hpp \
  utility/transport/transport_peer.hpp \
  utility/transport/transport_protocol.hpp \
  utility/transport/rfcomm/rfcomm.hpp \
  utility/transport/transport.hpp \
  utility/transport/asio/bluetooth_endpoint.hpp \
  utility/transport/asio/rfcomm.hpp \
  utility/transport/transport_listener.hpp \
  utility/transport/asio/asio_io_service.h \
  utility/transport/tcpip/protlib/threadsafe_db.h \
  utility/transport/tcpip/protlib/configuration.h \
  utility/transport/tcpip/protlib/ie.h \
  utility/transport/tcpip/protlib/llhashers.h \
  utility/transport/tcpip/protlib/fqueue.h \
  utility/transport/tcpip/protlib/assocdata_uds.h \
  utility/transport/tcpip/protlib/address.h \
  utility/transport/tcpip/protlib/logfile.h \
  utility/transport/tcpip/protlib/timer.h \
  utility/transport/tcpip/protlib/queuemanager.h \
  utility/transport/tcpip/protlib/messages.h \
  utility/transport/tcpip/protlib/assocdata.h \
  utility/transport/tcpip/protlib/protlib_types.h \
  utility/transport/tcpip/protlib/tp_over_tcp.h \
  utility/transport/tcpip/protlib/tp_over_udp.h \
  utility/transport/tcpip/protlib/tp.h \
  utility/transport/tcpip/protlib/threads.h \
  utility/transport/tcpip/protlib/connectionmap.h \
  utility/transport/tcpip/protlib/timer_module.h \
  utility/transport/tcpip/protlib/fastqueue.h \
  utility/transport/tcpip/protlib/tperror.h \
  utility/transport/tcpip/protlib/network_message.h \
  utility/transport/tcpip/protlib/setuid.h \
  utility/transport/tcpip/protlib/cleanuphandler.h \
  utility/transport/tcpip/protlib/connectionmap_uds.h

#------------> utility :: messages

libariba_la_SOURCES += \
  utility/messages/Message.cpp \
  utility/messages/MessageProvider.cpp \
  utility/messages/MessageReceiver.cpp \
  utility/messages/MessageSender.cpp \
  utility/messages/TextMessage.cpp

nobase_libariba_la_HEADERS += \
  utility/messages/_namespace.h \
  utility/messages/Message.h \
  utility/messages/MessageProvider.h \
  utility/messages/MessageReceiver.h \
  utility/messages/MessageSender.h \
  utility/messages/MessageUtilities.h \
  utility/messages/TextMessage.h

#------------> utility :: misc

libariba_la_SOURCES += \
  utility/misc/Helper.cpp \
  utility/misc/sha1.cpp \
  utility/misc/StringFormat.cpp

nobase_libariba_la_HEADERS += \
  utility/misc/Demultiplexer.hpp \
  utility/misc/Helper.h \
  utility/misc/KeyMapping.hpp \
  utility/misc/sha1.h \
  utility/misc/StringFormat.h

#------------> utility :: serialization

libariba_la_SOURCES += \
  utility/serialization/Serialization.cpp \
  utility/serialization/TestSerialization.cpp

nobase_libariba_la_HEADERS += \
  utility/serialization/DataUtilities.hpp \
  utility/serialization/Serialization.hpp \
  utility/serialization/TestSerialization.h \
  utility/serialization/Data.hpp \
  utility/serialization/DataStream.hpp

#------------> utility :: system

libariba_la_SOURCES += \
  utility/system/BlockingMethod.cpp \
  utility/system/EnterMethod.cpp \
  utility/system/SystemEvent.cpp \
  utility/system/SystemEventListener.cpp \
  utility/system/SystemEventType.cpp \
  utility/system/SystemQueue.cpp \
  utility/system/Timer.cpp \
  utility/system/StartupWrapper.cpp

nobase_libariba_la_HEADERS += \
  utility/system/BlockingMethod.h \
  utility/system/EnterMethod.h \
  utility/system/SystemEvent.h \
  utility/system/SystemEventListener.h \
  utility/system/SystemEventType.h \
  utility/system/SystemQueue.h \
  utility/system/Timer.h \
  utility/system/StartupWrapper.h \
  utility/system/StartupInterface.h

#------------> utility :: types

libariba_la_SOURCES += \
  utility/types/Address.cpp \
  utility/types/Identifier.cpp \
  utility/types/LinkID.cpp \
  utility/types/Locator.cpp \
  utility/types/NodeID.cpp \
  utility/types/PeerID.cpp \
  utility/types/OverlayParameterSet.cpp \
  utility/types/QoSParameterSet.cpp \
  utility/types/SecurityParameterSet.cpp \
  utility/types/ServiceID.cpp \
  utility/types/SpoVNetID.cpp

nobase_libariba_la_HEADERS += \
  utility/types/_namespace.h \
  utility/types/Address.h \
  utility/types/Data.hpp \
  utility/types/Identifier.h \
  utility/types/LinkID.h \
  utility/types/Locator.h \
  utility/types/NodeID.h \
  utility/types/PeerID.h \
  utility/types/OverlayParameterSet.h \
  utility/types/QoSParameterSet.h \
  utility/types/SecurityParameterSet.h \
  utility/types/ServiceID.h \
  utility/types/SpoVNetID.h

#------------> utility :: visual

libariba_la_SOURCES += \
  utility/visual/OvlVis.cpp \
  utility/visual/ServerVis.cpp \
  utility/visual/DddVis.cpp

nobase_libariba_la_HEADERS += \
  utility/visual/OvlVis.h \
  utility/visual/ServerVis.h \
  utility/visual/DddVis.h
