#pragma once
#include <chrono>
using Duration = std::chrono::duration<uint64_t, std::nano>;
using Timestamp = std::chrono::time_point<std::chrono::high_resolution_clock, Duration>;
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
/// @return a timestamp for the current time
inline const Timestamp now()
{
	return std::chrono::time_point_cast<Duration>(Timestamp::clock::now());
}
