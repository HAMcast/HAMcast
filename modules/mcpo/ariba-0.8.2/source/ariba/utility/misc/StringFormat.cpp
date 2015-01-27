
#include "StringFormat.h"

#include "boost/xpressive/xpressive.hpp"

namespace ariba {
namespace utility {
namespace string_format {

using namespace boost::xpressive;

// regex: string
const sregex rstring = '"' >> keep(*~(boost::xpressive::set = '"'))
		>> '"';

// regex: base64 encoding
const sregex rbase64 = '!' >> +(range('a', 'z') | range('A', 'Z')
		| range('0', '9') | '/' | '+') >> *(boost::xpressive::set = '=');

// regex: raw alphabet
const sregex rchars = +(range('a', 'z') | range('A', 'Z'));

// regex: integer
const sregex rint = '0' | (range('1', '9') >> !(range('0', '9')));

// regex: binary label
const sregex rlabel = rchars | rstring | rbase64;

// regex: dot separated identifier
const sregex rid = rlabel >> *('.' >> rlabel) >> *('.' >> rint);

// regex: "leaf" data
const sregex rdata = !(boost::xpressive::set = '!') >> '{'
		>> *(keep(+~(boost::xpressive::set = '{', '}')) | by_ref(rdata))
		>> '}';

// regex: fields
const sregex rfield_label = rlabel >> '=';
const sregex rfield = !rfield_label >> (rid | rdata);
const sregex rfields = '(' >> rfield >> *(',' >> rfield) >> ')';

// regex objects
const sregex robject_data = (rdata | rfields);
const sregex robject_id = rid;
const sregex robject = robject_id >> robject_data;
const sregex robjects = robject >> *(',' >> robject);

}}}
