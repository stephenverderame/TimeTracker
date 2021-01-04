#include "FileStreamProvider.h"
#include <unordered_map>
#include <fstream>
#include <filesystem>
#include <iomanip>
#include <sstream>

const char * extension = ".st";

struct FileStreamProvider::impl {
	std::string dirName;
	std::unordered_map<std::string, std::shared_ptr<StreamWrapper>> streams;

	impl(std::string&& dirName) : dirName(std::move(dirName)) {};
};
/// @return true if c cannot be in a file name
inline bool isIllegalCharacter(char c)
{
	return c == ' ' || c == '|' || c == '\\' || c == '*' || c == '/'
		|| (c >= 58 && c <= 63);
}
/**
 * Makes the filename appropriate for the file system by using url encoding
 */
std::string sanitizeForFileSystem(const std::string& name)
{
	std::stringstream ss;
	for (auto c : name)
	{
		if (isIllegalCharacter(c))
			ss << '%' << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(c);
		else if (c == '%')
			ss << "%%";
		else
			ss << c;
	}
	return ss.str();
}

inline std::string streamNameToFileName(const std::string& dirName, const char* name)
{
	const std::string s = sanitizeForFileSystem(name);
	return dirName + '/' + s + extension;
}

inline void createFile(const std::string& dirName, const char* streamName) {
	std::ofstream(streamNameToFileName(dirName, streamName), std::ios::binary);
}

inline auto makeStream(const std::string& dirName, const char* taskName)
{
	return std::make_shared<StreamWrapper>(std::make_unique<std::fstream>(streamNameToFileName(dirName, taskName),
		std::ios::binary | std::ios::in | std::ios::out));
}
/**
 * Unsanitizes a sanitized name
 */
std::string decodeSanitation(const std::string& sanitizedName)
{
	std::stringstream ss;
	for (size_t i = 0; i < sanitizedName.size();)
	{
		if (sanitizedName[i] == '%' && i + 2 < sanitizedName.size() && sanitizedName[i + 1] != '%') {
			char hexNum[] = { sanitizedName[i + 1], sanitizedName[i + 2], '\0' };
			const int val = std::stoi(hexNum, nullptr, 16);
			ss << static_cast<char>(val);
			i += 3;
		}
		else {
			ss << sanitizedName[i++];
		}
	}
	return ss.str();
}

std::shared_ptr<StreamWrapper> FileStreamProvider::getStream(const char* name)
{
	auto it = pimpl->streams.find(name);
	if (it != pimpl->streams.end()) {
		return it->second;
	}
	if (!doesStreamExist(name)) createFile(pimpl->dirName, name);
	auto str = makeStream(pimpl->dirName, name);
	pimpl->streams[name] = str;
	return str;
}

bool FileStreamProvider::doesStreamExist(const char* name) const
{
	return std::filesystem::exists(streamNameToFileName(pimpl->dirName, name));
}

inline void removeTask(const std::string& dirName, const char* taskName)
{
	std::filesystem::remove(streamNameToFileName(dirName, taskName));
}

void FileStreamProvider::clearStreamData(const char* name)
{
	if (!doesStreamExist(name)) return;
	auto it = pimpl->streams.find(name);
	if (it != pimpl->streams.end()) {
		{
			StreamWrapper w(std::make_unique<std::fstream>(pimpl->dirName + "/tmp"));
			it->second->swap(w);
		}
		removeTask(pimpl->dirName, name);
		createFile(pimpl->dirName, name);
		auto w2 = makeStream(pimpl->dirName, name);
		it->second->swap(*w2);
	}
	else
		removeTask(pimpl->dirName, name);
}

std::vector<std::string> FileStreamProvider::getAllStreamNames() const
{
	std::filesystem::directory_iterator dir(pimpl->dirName);
	auto fs = std::filesystem::current_path();
	std::vector<std::string> streams;
	for (const auto& f : dir)
	{
		const auto path = f.path();
		if (path.extension() == extension)
			streams.push_back(decodeSanitation(path.stem().string()));
	}
	return streams;
}

FileStreamProvider::~FileStreamProvider() = default;

FileStreamProvider::FileStreamProvider(const char* topic) : 
	pimpl(std::make_unique<impl>(sanitizeForFileSystem(topic))) {
	if (!std::filesystem::exists(pimpl->dirName)) {
		std::filesystem::create_directory(pimpl->dirName);
	}
}
