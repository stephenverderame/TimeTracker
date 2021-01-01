#include "FileStreamProvider.h"
#include <unordered_map>
#include <fstream>
#include <filesystem>

struct FileStreamProvider::impl {
	std::unordered_map<std::string, std::shared_ptr<StreamWrapper>> streams;
};

inline void createFile(const char* name) {
	std::ofstream(name, std::ios::binary);
}

std::shared_ptr<StreamWrapper> FileStreamProvider::getStream(const char* name)
{
	auto it = pimpl->streams.find(name);
	if (it != pimpl->streams.end()) {
		return it->second;
	}
	if (!doesStreamExist(name)) createFile(name);
	auto str = std::make_shared<StreamWrapper>(std::make_unique<std::fstream>(name,
		std::ios::binary | std::ios::in | std::ios::out));
	pimpl->streams[name] = str;
	return str;
}

bool FileStreamProvider::doesStreamExist(const char* name) const
{
	return std::filesystem::exists(name);
}

void FileStreamProvider::clearStreamData(const char* name)
{
	if (!doesStreamExist(name)) return;
	auto it = pimpl->streams.find(name);
	if (it != pimpl->streams.end()) {
		{
			StreamWrapper w(std::make_unique<std::fstream>("tmp"));
			it->second->swap(w);
		}
		std::filesystem::remove(name);
		createFile(name);
		StreamWrapper w2(std::make_unique<std::fstream>(name, 
			std::ios::binary | std::ios::in | std::ios::out));
		it->second->swap(w2);
	}
	else
		std::filesystem::remove(name);
}

FileStreamProvider::~FileStreamProvider() = default;

FileStreamProvider::FileStreamProvider() : pimpl(std::make_unique<impl>()) {}
