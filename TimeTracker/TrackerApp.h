#pragma once
#include "TimeTracker.h"
#include "AppTimes.h"
class TaskTracker : public TimeTracker {
	struct impl;
	std::unique_ptr<impl> pimpl;
public:
	TaskTracker(std::unique_ptr<class LoggerGateway>&& database);
	~TaskTracker();
	void startTask(const char* task) override;
	void pauseTask(const char* task) override;
	Duration taskElapsed(const char* task, Timestamp start, Timestamp end) const override;

	TaskTracker(const TaskTracker&) = delete;
	TaskTracker& operator=(const TaskTracker&) = delete;


};