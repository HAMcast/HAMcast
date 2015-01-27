
#ifndef STRINGFORMAT_H_
#define STRINGFORMAT_H_

#include "boost/xpressive/xpressive.hpp"

namespace ariba {
namespace utility {
namespace string_format {

using boost::xpressive::sregex;

class regex_nav {
private:
	typedef boost::xpressive::smatch _match;
	typedef _match::nested_results_type nested_results;
	typedef nested_results::const_iterator nested_iterator;
	const _match& match;

public:
	regex_nav(const _match& match) :
		match(match) {
	}

	regex_nav() :
		match(*((const _match*) NULL)) {
	}

	bool matched() const {
		return &match != NULL;
	}

	regex_nav operator[] (const sregex& type) const {
		const nested_results& nr = match.nested_results();
		for (nested_iterator i = nr.begin(); i != nr.end(); i++) {
			if (i->regex_id() == type.regex_id()) return regex_nav(*i);
		}
		return regex_nav();
	}

	regex_nav operator[](int index) const {
		const nested_results& nr = match.nested_results();
		for (nested_iterator i = nr.begin(); i != nr.end() && index >= 0; i++) {
			if (index == 0) return regex_nav(*i);
			index--;
		}
		return regex_nav();
	}

	int size() const {
		return match.nested_results().size();
	}

	std::string str() const {
		if (!matched()) return std::string("<no match>");
		return match[0].str();
	}
};

// regex: string
extern const sregex rstring;

// regex: base64 encoding
extern const sregex rbase64;

// regex: raw alphabet
extern const sregex rchars;

// regex: integer
extern const sregex rint;

// regex: binary label
extern const sregex rlabel;

// regex: dot separated identifier
extern const sregex rid;

// regex: "leaf" data
extern const sregex rdata;

// regex: fields
extern const sregex rfield_label;
extern const sregex rfield;
extern const sregex rfields;

// regex: objects
extern const sregex robject_data;
extern const sregex robject_id;
extern const sregex robject;
extern const sregex robjects;

}}}

#endif /* STRINGFORMAT_H_ */
