/******************************************************************************\
 *  _   ___     ____  __               _                                      *
 * | | | \ \___/ /  \/  | ___ __ _ ___| |_                                    *
 * | |_| |\     /| |\/| |/ __/ _` / __| __|                                   *
 * |  _  | \ - / | |  | | (_| (_| \__ \ |_                                    *
 * |_| |_|  \_/  |_|  |_|\___\__,_|___/\__|                                   *
 *                                                                            *
 * This file is part of the HAMcast project.                                  *
 *                                                                            *
 * HAMcast is free software: you can redistribute it and/or modify            *
 * it under the terms of the GNU Lesser General Public License as published   *
 * by the Free Software Foundation, either version 3 of the License, or       *
 * (at your option) any later version.                                        *
 *                                                                            *
 * HAMcast is distributed in the hope that it will be useful,                 *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                       *
 * See the GNU Lesser General Public License for more details.                *
 *                                                                            *
 * You should have received a copy of the GNU Lesser General Public License   *
 * along with HAMcast. If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                            *
 * Contact: HAMcast support <hamcast-support@informatik.haw-hamburg.de>       *
\******************************************************************************/

INTRODUCTION

In this file you find all information to build and run the HAMcast
prototype provided in this package. Just read and follow the steps below.
However, if you are interested in more details on HAMcast take a look into
the 'docs' directory, it contains a current version of the IRTF draft for
the common multicast API and a doxygen code documentation of libhamcast,
including code examples.

You may also visit our developers webpage <hamcast.realmv6.org/developers>.


RELEASE FOLDER STRUCTURE AND CONTENT

Release directory structure:

  hamcast_core/
    -docs               -- documentation
    -libhamcast         -- HAMcast C++ API library
    -middleware         -- HAMcast middleware
    +modules/           -- all multicast technology modules
        -bidirsam       -- BIDIR-SAM ALM
        -ipm            -- IP(v4/v6) multicast
        -loopback       -- dummy module with local loop
        -scribe         -- Scribe ALM
        -void           -- dummy module dropping all data
    +programs/          -- examples software tools
        -hc_chat        -- simple chat client
        -hc_status      -- show state or test of the middleware
        -img            -- a simple IMG daemon
        -monitoring     -- frameworking to monitor hybrid multicast networks
    +testing/           -- performance tests
        -test_receive_performance
        -test_send_performance
    +utils/             -- utils and shared libs
        -chimera        -- pastry-based p2p unicast overlay network


REQUIREMENTS

To build and run HAMcast certain tools and libraries must be available.

 * General dependencies:
    - C/C++ Compiler (gcc, g++, clang)
    - C/C++ standard Libraries
    - cmake tool chain (v2.6 or higher)
    - make
    - Boost Developers Libraries (v1.42 or higher)
      Ubuntu shortcut: sudo apt-get install build-essential libboost-all-dev cmake

 * IP module dependencies:
    - PCAP Developers Library (optional, for service discovery)
      Ubuntu shortcut: sudo apt-get install libpcap-dev

 * BIDIR-SAM + Scribe module dependencies:
    - OpenSSL Developers Library
      Ubuntu shortcut: sudo apt-get install libssl-dev

For MacOS-X please check The MacPorts Project <http://www.macports.org/>,
if any library or tool is missing in the standard repositories. The Boost
Library (source code) is also available on <http://www.boost.org/>.


BUILD HAMCAST

__General__

Our build toolchain uses the cmake framework. For all build processes we
recommend an out-of-source build, this complies to cmake BCP. Our core
release contains a main CMakeLists.txt, that recursivly builds libhamcast,
middleware and modules. To build the full core release, follow these
steps:

    - change to core release folder hamcast_core/
    - create a build folder, e.g. hamcast_core/build
    - change to build folder
    - run cmake:
        cmake ../
      or for debugging
        cmake -DCMAKE_BUILD_TYPE=Debug ../

    - If no errors occure cmake creates a Makefile, run in build folder:
        make

Afterwards all libraries (libhamcast and modules) are stored in
hamcast_core/lib and the middleware in hamcast_core/bin You can also compile
each part of HAMcast separately, see instructions below.

__libhamcast__

Libhamcast is the core library of the HAMcast prototype and must be build
first. Follow these steps to compile libhamcast:

    - change to folder libhamcast/, this is where CMakeLists.txt lies
    - create a build folder, e.g. libhamcast/build
    - change to build folder and run:
        cmake ../
    - If no errors occure cmake creates a Makefile, now run in build folder:
        make
    - Compiled library is stored in build folder.

Note: you may copy libhamcast.so* files to source folder, then you only
have to specify HAMCAST_INCLUDE_PATH for other builds that depend on
libhamcast and its header files (see below).

__middleware, modules and others__

All other components of the HAMcast prototype as well as all programs
depend on libhamcast, its header files and optional libraries. For further
information have a look at the README file in the corresponding source
folder. The general build steps are as follows:

    - change to <source-dir>/, this is where CMakeLists.txt lies
    - create a build folder, e.g. <source-dir>/build
    - change to build folder
    - if HAMcast library and header are installed to standard folders
      (/usr/lib/, /usr/include), run:
        cmake ../
    - otherwise specify header and library location as follows:
        cmake -DHAMCAST_INCLUDE_PATH=<path-to-headers> \
              -DHAMCAST_LIBRARY_PATH=<path-to-libhamcast> ../

    - NOTE: <path-to-headers> must point to directory where
      hamcast/hamcast.hpp lies.
    - If no errors occure cmake creates a Makefile, now run in build folder:
        make
    - Compiled binary or library are stored in build folder


RUN HAMCAST

To run the HAMcast middleware you first need to create a valid
configuration file named 'middleware.ini'. There is an example file
'middleware/middleware.ini.example', copy/rename it to 'bin/middleware.ini'.
Then uncomment the lines for a technology module you want to use by removing
preceding ';'. For Linux use the *.so files, and *.dylib for MacOS 
respectively. You also have to uncomment corresponding section name given
in '[<module name>]' otherwise the middleware is unable to parse the
configuration.

Afterwards, you can start the middleware either by typing './middleware'
in 'hamcast_core/bin' folder or run the shell script 'run_hamcast.sh' in
'hamcast_core/'.

For further information on developing group communication applications
using HAMcast visit our developers webpage on <hamcast.realmv6.org>.
