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
inline auto streamSize(std::istream& str)
{
	str.seekg(0, std::ios::beg);
	auto first = str.tellg();
	str.seekg(0, std::ios::end);
	return str.tellg() - first;;
}
Timestamp addRecord(std::shared_ptr<StreamWrapper> stream, RecordType type)
{
	const auto currentTime = now();
	Record r;
	r.recordType = RecordType::end;
	auto& in = stream->getRead();
	if (!in.bad() && streamSize(in) >= sizeof(Record)) {
		in.seekg(-record_size, std::ios::end);
		in.read(reinterpret_cast<char*>(&r), record_size);
	}
	if (r.recordType != type) {
		auto& out = stream->getWrite();
		out.seekp(0, std::ios::end);
		out.write(reinterpret_cast<const char*>(&currentTime), sizeof(currentTime));
		out.write(reinterpret_cast<const char*>(&type), sizeof(type));
	}
	return currentTime;
}

Timestamp StreamLogger::logBegin(const char* task)
{

	return addRecord(streams->getStream(task), RecordType::start);
}

Timestamp StreamLogger::logEnd(const char* task)
{
	return addRecord(streams->getStream(task), RecordType::end);
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
Record recordAt(std::shared_ptr<ReaderStream> f, std::remove_cv_t<decltype(record_size)> recordIndex)
{
	Record ret;
	f->getRead().seekg(indexToAbsolutePosition(recordIndex));
	f->getRead().read(reinterpret_cast<char*>(&ret), record_size);
	return ret;
}
/// Requires file to be readable
/// @return the amount of filesystem logger records in the file
/// @throw runtime_error if file is corrupted or incorrect format
auto recordCount(std::shared_ptr<ReaderStream> f)
{
	if (f->size == ReaderStream::bad_cache) {
		auto sz = streamSize(f->getRead());
		if (sz % record_size) throw std::runtime_error("Corrupted file!");
		f->size = sz / record_size;
	}
	return f->size;

}
/// @return an index (from 0 to the amount of records in the file) of the timestamp or where it would be
/// Requires there is at least 1 record
auto indexOf(std::shared_ptr<ReaderStream> f, const Timestamp& t)
{
	const auto records = recordCount(f);
	std::remove_cv_t<decltype(records)> lo = 0;
	auto hi = records - 1;
	while (lo <= hi)
	{
		const auto mid = (hi - lo) / 2 + lo;
		const auto time = recordAt(f, mid).t;
		if (t < time)
			hi = mid - 1;
		else if (t > time)
			lo = mid + 1;
		else
			return mid;
	}
	return std::max(std::min(lo, hi), 0ll);
}
/**
 * Gets a list of sessions from a list of records
 * If any records are cut off by the start/end time, they are turned into a session of the duration that 
 * is within the start and end times
 * @param r stream records to be converted to sessions
 * @param startTime the startTime of the records, if the first record is an end record, uses the startTime to construct a complete session
 * @param endTime the endTime of the records, if the last record is a start record, uses this to construct a complete session
 */
std::vector<Session> recordListToSessionList(const std::vector<Record>& r, const Timestamp& startTime, 
	const Timestamp& endTime)
{
	std::vector<Session> sessions;
	const Timestamp * start = nullptr;
	for (size_t i = 0; i < r.size(); ++i)
	{
		const auto& record = r[i];
		if (i == 0 && record.recordType == RecordType::end && 
			record.t > startTime) {
			sessions.emplace_back(startTime, record.t - startTime);
		}
		else if (i == r.size() - 1 && record.recordType == RecordType::start
			&& record.t < endTime) {
			sessions.emplace_back(record.t, std::min(endTime, now()) - record.t);
		}
		else if (start == nullptr && record.recordType == RecordType::start) {
			start = &record.t;
		}
		else if (start != nullptr && record.recordType == RecordType::end) {
			sessions.emplace_back(*start, record.t - *start);
			start = nullptr;
		}
	}
	return sessions;
}
void assertRecordsAreLogical(std::shared_ptr<ReaderStream> rd) {
	auto end = recordCount(rd);
	auto first = recordAt(rd, 0);
	for (auto i = 1ll; i < end; ++i)
	{
		const auto cur = recordAt(rd, i);
		if (cur.t < first.t) throw std::runtime_error("Bad timing");
		else if (cur.recordType == first.recordType) throw std::runtime_error("Bad records");
		first = cur;
	}
}

std::vector<Session> StreamLogger::timeSpentBetween(const Timestamp& start, const Timestamp& end, 
	const char* task) const
{
	auto str = streams->getStream(task);
	auto& in = str->getRead();
	if (!in.bad()) {
		const auto endIndex = [&]() { //inclusive
			auto endIndex = indexOf(str, end);
			if (recordAt(str, endIndex).t > end)
				--endIndex;
			return endIndex;
		}();
		const auto startIndex = [&]() { //inclusive
			auto s = indexOf(str, start);
			if (recordAt(str, s).t < start && s < recordCount(str) - 1)
				++s;
			return s;

		}();
		if (endIndex <= 0) return {};
//		assertRecordsAreLogical(str);
		std::vector<Record> records(endIndex + 1 - startIndex);
		in.seekg(indexToAbsolutePosition(startIndex));
		in.read(reinterpret_cast<char*>(&records[0]), 
			indexToAbsolutePosition(endIndex) + record_size - indexToAbsolutePosition(startIndex));
		return recordListToSessionList(records, start, end);
	}
	throw FnfException(task);
}

size_t StreamLogger::numLogs(const char* task) const
{
	auto in = streams->getStream(task);
	if (in->getRead().bad()) throw std::runtime_error("Cannot open file");
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
	auto str = streams->getStream(task);	
	if (!str->getRead().bad()) {
		const auto count = recordCount(str);
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

std::vector<std::string> StreamLogger::listAllTasks() const
{
	return streams->getAllStreamNames();
}
