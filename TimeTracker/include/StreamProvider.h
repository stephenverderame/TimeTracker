#pragma once
#include <istream>
#include <memory>
#include <vector>
class StreamProvider {
public:
	virtual std::shared_ptr<class StreamWrapper> getStream(const char* name) = 0;
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
	static const long long bad_cache = -1;
	/// Implementation defined size of stream or bad_cache if it must be recalculated
	long long size = bad_cache;
public:
	virtual std::istream& getRead() = 0;
};

class WriterStream {
public:
	virtual std::ostream& getWrite() = 0;
};

class StreamWrapper : public ReaderStream, WriterStream {
private:
	std::unique_ptr<std::iostream> stream;
public:
	std::istream& getRead() override { return *stream; }
	std::ostream& getWrite() override { 
		size = bad_cache;
		return *stream; 
	}
	StreamWrapper(std::unique_ptr<std::iostream>&& str) : stream(std::move(str)) {};
	StreamWrapper(StreamWrapper&& sm) noexcept : stream(std::move(sm.stream)) {}
	StreamWrapper& operator=(std::unique_ptr<std::iostream>&& str);
	void swap(StreamWrapper& other) noexcept;

};