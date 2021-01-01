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

void TaskTracker::startTask(const char* task)
{
	pimpl->db->logBegin(task);
}

void TaskTracker::pauseTask(const char* task)
{
	pimpl->db->logEnd(task);
}

Duration TaskTracker::taskElapsed(const char* task, Timestamp start, Timestamp end) const
{
	const auto v = pimpl->db->timeSpentBetween(start, end, task);
	Duration d = Duration::zero();
	for (auto& ses : v) {
		if (ses.start < end)
			d += ses.duration;
		else
			break;
	}
	return d;
}
