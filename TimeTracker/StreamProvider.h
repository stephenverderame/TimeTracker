#pragma once
#include <istream>
#include <memory>
#include <vector>
class StreamProvider {
public:
	virtual class StreamMetadata& getStream(const char* name) = 0;
	virtual bool doesStreamExist(const char* name) const = 0;
	virtual void clearStreamData(const char* name) = 0;
	virtual ~StreamProvider() = default;
};
enum class StreamType {
	memory, file
};

std::shared_ptr<StreamProvider> makeStreamProvider(StreamType type);

class ReaderStream {
public:
	virtual std::istream& getRead() = 0;
	virtual const std::vector<char>& getBuffer() const = 0;
	virtual const size_t bufferSize() const = 0;
};

class WriterStream {
public:
	virtual std::ostream& getWrite() = 0;
};

class StreamMetadata : public ReaderStream, WriterStream {
private:
	std::unique_ptr<std::iostream> stream;
	mutable std::vector<char> buffer;
	mutable std::streampos bufferPos;
public:
	std::istream& getRead() override { return *stream; }
	std::ostream& getWrite() override { 
		buffer.clear();
		return *stream; 
	}
	const std::vector<char>& getBuffer() const override;
	const size_t bufferSize() const override { return buffer.size(); }
	StreamMetadata(std::unique_ptr<std::iostream>&& str) : stream(std::move(str)), bufferPos(std::ios::beg) {};
	StreamMetadata(StreamMetadata&& sm) noexcept : stream(std::move(sm.stream)), buffer(std::move(sm.buffer)), 
		bufferPos(sm.bufferPos) {}
	StreamMetadata& operator=(std::unique_ptr<std::iostream>&& str);

};