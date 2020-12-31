#include "FNFException.h"
#include <sstream>
const char* FnfException::what() const noexcept
{
	if (msg.empty()) {
		try {
			std::stringstream ss;
			ss << "File: " << fileName << " could not be found";
			msg = ss.str();
		}
		catch (std::exception&) {
			return fileName.c_str();
		}
	}
	return msg.c_str();
}
