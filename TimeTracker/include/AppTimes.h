#pragma once
#include <chrono>
#include <string>

using Duration = std::chrono::duration<uint64_t, std::nano>;
using Timestamp = std::chrono::time_point<std::chrono::system_clock, Duration>;
/*
using Duration = std::chrono::duration<uint64_t, std::milli>;
using Timestamp = std::chrono::time_point<std::chrono::system_clock, Duration>;
*/
struct Session {
	Timestamp start;
	Duration duration;
	Session() = default;
	Session(Timestamp start, Duration dur) : start(start), duration(dur) {}

	bool operator==(const Session& s) const
	{
		return start == s.start && duration == s.duration;
	}
};
/// @return a UTC timestamp for the current time
inline const Timestamp now()
{
	return std::chrono::time_point_cast<Duration>(Timestamp::clock::now());
}
/**
 * Parses a UTC timestamp into a string following the format
 * If fmt is not specified, a default one is used
 */
std::string printTimestamp(Timestamp t, const char * fmt = nullptr);
/**
 * Reads a local time string and converts it to a UTC timestamp
 * Requires string be in the format of fmt, which is the default one if not specified
 */
Timestamp readTimestamp(const std::string& s, const char * fmt = nullptr);

/**
 * Tries to determine a date format from the given string
 * Prioritizes american mm/dd/yy over dd/mm/yy if ambiguous
 * @param s a string containing only a possible date or datetime with unknown formatting
 * @return a string that describes the date formatting with POSIX date codes
 */
std::string guessDateFormat(const std::string& s);

std::string printDuration(Duration d);