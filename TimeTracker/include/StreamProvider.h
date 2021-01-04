#pragma once
#include <istream>
#include <memory>
#include <vector>
#include <string>
class StreamProvider {
public:
	virtual std::shared_ptr<class StreamWrapper> getStream(const char* name) = 0;
	virtual bool doesStreamExist(const char* name) const = 0;
	virtual void clearStreamData(const char* name) = 0;
	virtual std::vector<std::string> getAllStreamNames() const = 0;
	virtual ~StreamProvider() = default;
};
enum class StreamType {
	memory, file
};
/**
 * Makes a stream provider of the designated type
 * @param streamTopic the root name of all provided streams
 */
std::shared_ptr<StreamProvider> makeStreamProvider(StreamType type, const char * streamTopic);

class ReaderStream {
public:
	static const long long bad_cache = -1;
	/// Implementation defined size of stream or bad_cache if it must be recalculated
	long long size = bad_cache;
public:
	virtual std::istream& getRead() = 0;
protected:
	void inline invalidateCache() { size = bad_cache; }
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
		invalidateCache();
		return *stream; 
	}
	StreamWrapper(std::unique_ptr<std::iostream>&& str) : stream(std::move(str)) {};
	StreamWrapper(StreamWrapper&& sm) noexcept : stream(std::move(sm.stream)) {}
	StreamWrapper& operator=(std::unique_ptr<std::iostream>&& str);
	void swap(StreamWrapper& other) noexcept;

};