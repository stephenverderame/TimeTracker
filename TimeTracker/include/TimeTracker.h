#pragma once
#include <memory>
#include "AppTimes.h"
class TimeTracker {
public:
	virtual ~TimeTracker() = default;
	virtual void startTask(const char* task) = 0;
	virtual void pauseTask(const char* task) = 0;
	virtual Duration taskElapsed(const char* task, Timestamp start, Timestamp end) const = 0;
};
std::unique_ptr<TimeTracker> getTracker();