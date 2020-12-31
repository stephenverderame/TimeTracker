#include "StreamProvider.h"
#include "MemoryStreamProvider.h"
#include <istream>
#include <vector>
std::shared_ptr<StreamProvider> makeStreamProvider(StreamType type)
{
	switch (type) {
	case StreamType::memory:
		return std::make_shared<MemStreamProvider>();
	default:
		throw std::runtime_error("Unknown stream type");
	}
}
const std::vector<char>& StreamMetadata::getBuffer() const 
{
//	stream->sync();
//	stream->seekg(bufferPos);
	if (buffer.empty()) {
		buffer.insert(buffer.end(), std::istream_iterator<char>(*stream), std::istream_iterator<char>());
	}
//	bufferPos = stream->tellg();
	return buffer;
}
StreamMetadata& StreamMetadata::operator=(std::unique_ptr<std::iostream>&& str)
{
	stream = std::move(str);
	return *this;
}