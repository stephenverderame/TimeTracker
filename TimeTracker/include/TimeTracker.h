#pragma once
#include <memory>
#include "AppTimes.h"
#include <vector>
#include <string>
class TimeTracker {
public:
	virtual ~TimeTracker() = default;
	virtual Timestamp startTask(const char* task) = 0;
	virtual Timestamp pauseTask(const char* task) = 0;
	virtual std::vector<Session> sessionsBetween(const char* task, Timestamp start, Timestamp end) const = 0;
	virtual std::vector<std::string> listTasks() const = 0;
	virtual void delTask(const char* task) = 0;
};
enum class TrackerType {
	live, test
};
std::unique_ptr<TimeTracker> getTracker(TrackerType type);

/**
 * Gets the time spent on the specified task during a period, broken down by period
 * Will not include 0 durations if no activity was done during that period
 * @param period the breakdown period to use. So if 1h is specified, this will return the time spent each hour from start to end
 * @return a vector of durations where each one represents the time spent during the given breakdown
 */
std::vector<std::pair<Timestamp, Duration>> timeEachPeriod(const TimeTracker& tracker, const char* task,
	Timestamp start, Timestamp end, Duration period);

Duration elapsedTime(const TimeTracker& tracker, const char* task, Timestamp start, Timestamp end);