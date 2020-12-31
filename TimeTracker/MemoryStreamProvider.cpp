#include "MemoryStreamProvider.h"
#include <string>
#include <unordered_map>
#include <sstream>

struct MemStreamProvider::impl {
	std::unordered_map<std::string, StreamMetadata> streams;
};

StreamMetadata& MemStreamProvider::getStream(const char* name)
{
	if (pimpl->streams.find(name) != pimpl->streams.end()) {
		return pimpl->streams.at(name);
	}
	else {
		auto p = pimpl->streams.emplace(name, std::make_unique<std::stringstream>()).first;
		return (*p).second;
	}
}

bool MemStreamProvider::doesStreamExist(const char* name) const
{
	return pimpl->streams.find(name) != pimpl->streams.end();
}

void MemStreamProvider::clearStreamData(const char* name)
{
	if (doesStreamExist(name))
		pimpl->streams.at(name) = std::make_unique<std::stringstream>();
}

MemStreamProvider::~MemStreamProvider() = default;

MemStreamProvider::MemStreamProvider() : pimpl(std::make_unique<impl>()) {};