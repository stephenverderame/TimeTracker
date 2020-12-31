#include "StreamLogger.h"
#include <vector>
#include "FNFException.h"
#include <istream>
#include <assert.h>
using namespace maybe;

//Invariant: file is always ordered start - end, start - end

enum class RecordType : int16_t {
	start = '  ',
	end = '\n\n'
};

#pragma pack(push, 1)
struct Record {
	Timestamp t;
	RecordType recordType = RecordType::end;
};
#pragma pack(pop)

constexpr auto record_size = static_cast<std::make_signed_t<decltype(sizeof(Record))>>(sizeof(Record));

void addRecord(StreamMetadata& stream, RecordType type)
{
	if (stream.getWrite().good()) {
		const auto currentTime = now();
		Record r;
		r.recordType = RecordType::end;
		auto& in = stream.getRead();
		auto& out = stream.getWrite();
		in.seekg(0, std::ios::end);
		if (in.tellg() >= record_size) {
			in.seekg(-record_size, std::ios::end);
			in.read(reinterpret_cast<char*>(&r), sizeof(r));
		}
		if (r.recordType != type) {
			out.seekp(0, std::ios::end);
			out.write(reinterpret_cast<const char*>(&currentTime), sizeof(currentTime));
			out.write(reinterpret_cast<const char*>(&type), sizeof(type));

			in.seekg(-record_size, std::ios::end);
			in.read(reinterpret_cast<char*>(&r), sizeof(r));
			if(r.recordType != type || r.t != currentTime) throw std::runtime_error("Bad read write");
		}
	}
	else throw std::runtime_error("Cannot open file");
}

void StreamLogger::logBegin(const char* task)
{

	addRecord(streams->getStream(task), RecordType::start);
}

void StreamLogger::logEnd(const char* task)
{
	addRecord(streams->getStream(task), RecordType::end);
}
/// @return the starting position of the record at the given index
inline auto indexToAbsolutePosition(decltype(record_size) index)
{
	return index * record_size;
}
/**
 * Seeks the stream to the specified record and reads the time there
 * @return the timestamp of the record at the index
 * Requires recordIndex be valid
 */
Record recordAt(ReaderStream& f, std::remove_cv_t<decltype(record_size)> recordIndex)
{
	f.getRead().seekg(indexToAbsolutePosition(recordIndex));
	Record ret;
	f.getRead().read(reinterpret_cast<char*>(&ret), sizeof(ret));
	return ret;
}
/// @return the amount of filesystem logger records in the file
/// @throw runtime_error if file is corrupted or incorrect format
auto recordCount(ReaderStream& f)
{
	if (true/*f.size == ReaderStream::bad_cache*/) {
		f.getRead().seekg(0, std::ios::end);
		const auto size = f.getRead().tellg();
		f.getRead().seekg(0, std::ios::beg);
		if (size % record_size) throw std::runtime_error("Corrupted file!");
		return size / record_size;
	}
//	return f.size;

}
/// @return an index (from 0 to the amount of records in the file) of the timestamp or where it would be
auto indexOf(ReaderStream& f, const Timestamp& t)
{
	const auto records = recordCount(f);
	std::remove_cv_t<decltype(records)> lo = 0;
	auto hi = records - 1;
	while (lo < hi)
	{
		const auto mid = (hi + lo) / 2;
		const auto time = recordAt(f, mid).t;
		if (t < time)
			hi = mid - 1;
		else if (t > time)
			lo = mid + 1;
		else
			return mid;
	}
	return hi;
}
std::vector<Session> recordListToSessionList(const std::vector<Record>& r)
{
	std::vector<Session> sessions;
	const Timestamp * start = nullptr;
	for (const auto& record : r)
	{
		if (start == nullptr && record.recordType == RecordType::start) {
			start = &record.t;
		}
		else if (start != nullptr && record.recordType == RecordType::end) {
			sessions.emplace_back(*start, record.t - *start);
			start = nullptr;
		}
	}
	return sessions;
}

std::vector<Session> StreamLogger::timeSpentSince(const Timestamp& start, const char* task) const
{
	auto& str = streams->getStream(task);
	if (str.getRead().good()) {
		const auto endIndex = [&]() { //inclusive
			auto endIndex = indexOf(str, now());
			if (recordAt(str, endIndex).recordType == RecordType::start && endIndex < recordCount(str) - 1)
				++endIndex;
			printf("End Index: %lld Size: %lld\n", endIndex, recordCount(str));
			return endIndex;
		}();
		const auto startIndex = [&]() { //inclusive
			printf("Start: %llu First: %llu\n", start.time_since_epoch().count(),
				recordAt(str, 0).t.time_since_epoch().count());
			auto s = indexOf(str, start);
//			while (s > recordCount(str)) s = indexOf(str, start);
			if (recordAt(str, s).recordType == RecordType::end && s > 0)
				--s;
			printf("Start Index: %lld Size: %lld\n", s, recordCount(str));
			return s;

		}();
		std::vector<Record> records(endIndex + 1 - startIndex);
		str.getRead().seekg(indexToAbsolutePosition(startIndex));
		str.getRead().read(reinterpret_cast<char*>(&records[0]), (indexToAbsolutePosition(endIndex) + record_size) -
			indexToAbsolutePosition(startIndex));
		printf("%llu\n", records.size());
		return recordListToSessionList(records);
	}
	throw FnfException(task);
}

size_t StreamLogger::numLogs(const char* task) const
{
	auto& in = streams->getStream(task);
	if (!in.getRead().good()) throw std::runtime_error("Cannot open file");
	return recordCount(in);
}

void StreamLogger::clearTask(const char* task)
{
	streams->clearStreamData(task);
}

bool StreamLogger::doesTaskExist(const char* task) const
{
	return streams->doesStreamExist(task);
}

const Maybe<Session> StreamLogger::sessionAt(const Timestamp& time, const char* task) const
{
	auto& str = streams->getStream(task);
	const auto count = recordCount(str);
	if (str.getRead().good()) {
		const auto index = indexOf(str, time);
		const auto r = recordAt(str, index);
		if (r.recordType == RecordType::start && index < count - 1
			&& time >= r.t) {
			const auto end = recordAt(str, index + 1);
			if(time <= end.t)
				return Session{r.t, end.t - r.t};
		}
		else if (r.recordType == RecordType::end && time <= r.t) {
			const auto start = recordAt(str, index - 1);
			if(time >= start.t)
				return Session{start.t, r.t - start.t};
		}
	}
	return Empty();
}
