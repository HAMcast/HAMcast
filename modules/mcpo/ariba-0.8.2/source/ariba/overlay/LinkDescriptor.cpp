#include "LinkDescriptor.h"

namespace ariba {
namespace overlay {

std::ostream& operator<<(std::ostream& s, const LinkDescriptor* ld ) {
	return s << ld->to_string();
}

std::ostream& operator<<(std::ostream& s, const LinkDescriptor& ld ) {
	return s << ld.to_string();
}

}} // ariba, overlay
