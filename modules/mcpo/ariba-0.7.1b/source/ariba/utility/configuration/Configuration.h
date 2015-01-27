// [License]
// The Ariba-Underlay Copyright
//
// Copyright (c) 2008-2009, Institute of Telematics, Universität Karlsruhe (TH)
//
// Institute of Telematics
// Universität Karlsruhe (TH)
// Zirkel 2, 76128 Karlsruhe
// Germany
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE INSTITUTE OF TELEMATICS ``AS IS'' AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE ARIBA PROJECT OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// The views and conclusions contained in the software and documentation
// are those of the authors and should not be interpreted as representing
// official policies, either expressed or implied, of the Institute of
// Telematics.
// [License]

#ifndef __CONFIGURATION_H
#define __CONFIGURATION_H

#include <boost/utility.hpp>
#include <string>
#include "ConfigFile.h"
#include "ariba/utility/logging/Logging.h"

using std::string;

namespace ariba {
namespace utility {

class Configuration : private boost::noncopyable {
	use_logging_h(Configuration);
private:
	ConfigFile* config;
	static string CONFIG_FILE;

protected:
	Configuration();
	~Configuration();

public:
	static Configuration& instance();
	static void setConfigFilename(string filename);
	static bool haveConfig();

	/**
	 * Check if a key exists in the configuration
	 */
	bool exists(const string& name);

	/**
	 * Read a value from the configuration
	 */
	template<class T>
	T read(string name){
		if( !exists(name) )
			logging_error( "configuration key not found: " + name );

		return config->read<T>(name.c_str());
	}

	/**
	 * Reparse the configuration for changes in
	 * the file to take effect
	 */
	void reload();
};

}} // namespace ariba, utility

#endif //__CONFIGURATION_H
