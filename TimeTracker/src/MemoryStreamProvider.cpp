#include "MemoryStreamProvider.h"
#include <string>
#include <unordered_map>
#include <sstream>

struct MemStreamProvider::impl {
	std::unordered_map<std::string, std::shared_ptr<StreamWrapper>> streams;
};

std::shared_ptr<StreamWrapper> MemStreamProvider::getStream(const char* name)
{
	if (pimpl->streams.find(name) != pimpl->streams.end()) {
		return pimpl->streams.at(name);
	}
	else {
		auto p = pimpl->streams.emplace(name, std::make_shared<StreamWrapper>(std::make_unique<std::stringstream>())).first;
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
		pimpl->streams.at(name) = std::make_shared<StreamWrapper>(std::make_unique<std::stringstream>());
}

std::vector<std::string> MemStreamProvider::getAllStreamNames() const
{
	std::vector<std::string> s;
	for (const auto& e : pimpl->streams) {
		s.push_back(e.first);
	}
	return s;
}

MemStreamProvider::~MemStreamProvider() = default;

MemStreamProvider::MemStreamProvider() : pimpl(std::make_unique<impl>()) {};