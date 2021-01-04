#pragma once
#include "TimeTracker.h"
#include "AppTimes.h"
class TaskTracker : public TimeTracker {
	struct impl;
	std::unique_ptr<impl> pimpl;
public:
	TaskTracker(std::unique_ptr<class LoggerGateway>&& database);
	~TaskTracker();
	Timestamp startTask(const char* task) override;
	Timestamp pauseTask(const char* task) override;
	std::vector<Session> sessionsBetween(const char* task, Timestamp start, Timestamp end) const override;
	std::vector<std::string> listTasks() const override;
	void delTask(const char* task) override;
	TaskTracker(const TaskTracker&) = delete;
	TaskTracker& operator=(const TaskTracker&) = delete;


};