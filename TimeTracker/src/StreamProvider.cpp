#include "StreamProvider.h"
#include "MemoryStreamProvider.h"
#include "FileStreamProvider.h"
#include <istream>
#include <vector>
std::shared_ptr<StreamProvider> makeStreamProvider(StreamType type, const char* streamTopic)
{
	switch (type) {
	case StreamType::memory:
		return std::make_shared<MemStreamProvider>();
	case StreamType::file:
		return std::make_shared<FileStreamProvider>(streamTopic);
	default:
		throw std::runtime_error("Unknown stream type");
	}
}
StreamWrapper& StreamWrapper::operator=(std::unique_ptr<std::iostream>&& str)
{
	stream = std::move(str);
	size = bad_cache;
	return *this;
}

void StreamWrapper::swap(StreamWrapper& other) noexcept
{
	stream.swap(other.stream);
	std::swap(size, other.size);
}
