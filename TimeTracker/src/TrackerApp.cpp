#include "TrackerApp.h"
#include "LoggerGateway.h"

struct TaskTracker::impl {
	std::unique_ptr<LoggerGateway> db;
	impl(std::unique_ptr<LoggerGateway>&& db) : db(std::move(db)) {}
	
	impl(const impl&) = delete;
	impl& operator=(const impl&) = delete;
};

TaskTracker::TaskTracker(std::unique_ptr<LoggerGateway>&& database) :
	pimpl(std::make_unique<impl>(std::move(database)))
{
}

TaskTracker::~TaskTracker() = default;

Timestamp TaskTracker::startTask(const char* task)
{
	return pimpl->db->logBegin(task);
}

Timestamp TaskTracker::pauseTask(const char* task)
{
	return pimpl->db->logEnd(task);
}

std::vector<Session> TaskTracker::sessionsBetween(const char* task, Timestamp start, Timestamp end) const
{
	return pimpl->db->timeSpentBetween(start, end, task);
}

std::vector<std::string> TaskTracker::listTasks() const
{
	return pimpl->db->listAllTasks();
}

void TaskTracker::delTask(const char* task)
{
	pimpl->db->clearTask(task);
}
