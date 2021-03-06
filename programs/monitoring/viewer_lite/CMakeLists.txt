cmake_minimum_required(VERSION 2.6)
project (monitoring_viewer_lite)

# Set up environment paths to cmake modules and libhamcast
set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set (MONITORING_UTILS_PATH ../utils)

# search for libs
find_package (Qt4 REQUIRED)
find_package (Libigraph REQUIRED)
find_package (LibHamcast REQUIRED)
find_package (Boost COMPONENTS date_time filesystem regex system thread REQUIRED)

link_directories ( ${Boost_LIBRARY_DIRS} )
include_directories ( . ${CMAKE_CURRENT_BINARY_DIR} ${HAMCAST_INCLUDE}
                        ${Boost_INCLUDE_DIRS} ${MONITORING_UTILS_PATH} )

set (mviewer_lite_SOURCES    ${MONITORING_UTILS_PATH}/http_message.cpp
                            table_widget.cpp update_thread.cpp
                         imagenode.cpp node_details.cpp
                         edge.cpp drawgraph.cpp
                         tcp_connection.cpp connect_dialog.cpp
                         monitor.cpp hamcast_node.cpp
                         hamcast_interface.cpp hamcast_group.cpp
                         mainwindow.cpp main.cpp)

set (mviewer_lite_HEADERS animationellipse.h drawgraph.h update_thread.hpp
                          imagenode.h mainwindow.h node_details.hpp
                          monitor.hpp tcp_connection.hpp
                          connect_dialog.hpp table_widget.hpp
                          ${MONITORING_UTILS_PATH}/http_message.hpp
)

set (mviewer_lite_FORMS mainwindow.ui connect_dialog.ui node_details.ui table_widget.ui)
set (mviewer_lite_RESOURCES images.qrc)

set (QT_USE_QTNETWORK TRUE)
set (QT_USE_QTOPENGL TRUE)
set (QT_USE_QTWEBKIT TRUE)
set (QT_USE_QTXML TRUE)

include ( ${QT_USE_FILE} )

QT4_WRAP_CPP (mviewer_lite_HEADERS_MOC ${mviewer_lite_HEADERS})
QT4_WRAP_UI (mviewer_lite_FORMS_HEADERS ${mviewer_lite_FORMS})
QT4_ADD_RESOURCES (mviewer_lite_RESOURCES_RCC ${mviewer_lite_RESOURCES})

add_executable (mviewer_lite	${mviewer_lite_SOURCES}
                            	${mviewer_lite_HEADERS_MOC}
                            	${mviewer_lite_FORMS_HEADERS}
                            	${mviewer_lite_RESOURCES_RCC})

target_link_libraries (mviewer_lite	${QT_LIBRARIES}
                                	${QT_QTNETWORK_LIBRARIES}
                                	${QT_QTOPENGL_LIBRARIES}
                                	${QT_QTWEBKIT_LIBRARIES}
                                	${QT_QTXML_LIBRARIES}
                                	${IGRAPH_LIBRARY}
                                	${HAMCAST_LIBRARY} 
                                	${Boost_DATE_TIME_LIBRARY}
                                	${Boost_SYSTEM_LIBRARY}
                                	${Boost_FILESYSTEM_LIBRARY}
                                	${Boost_REGEX_LIBRARY}
                                	${Boost_THREAD_LIBRARY})
