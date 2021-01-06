#include "TimeTracker.h"
#include "TrackerApp.h"
#include "StreamLogger.h"
#include "StreamProvider.h"
#include <algorithm>
#include <numeric>
#include <regex>

std::unique_ptr<TimeTracker> getTracker(TrackerType t)
{
	switch (t) {
	case TrackerType::test:
		return std::make_unique<TaskTracker>(makeDBGateway(DBType::test));
	case TrackerType::live:
		return std::make_unique<TaskTracker>(makeDBGateway(DBType::local));
	default:
		throw std::runtime_error("Unknown app type");
	}
}

bool sessionInPeriod(const Session& session, const Timestamp& periodStart, const Timestamp& periodEnd)
{
	return !(session.start + session.duration <= periodStart || session.start >= periodEnd);
}

Duration chunkOfSession(const Session& session, const Timestamp& periodStart, const Timestamp& periodEnd)
{
	if (sessionInPeriod(session, periodStart, periodEnd))
		return std::min(session.start + session.duration, periodEnd) - 
		std::max(periodStart, session.start);
	else
		return std::chrono::nanoseconds(0);
}

std::vector<std::string> tasksMatching(const TimeTracker& tracker, const char* taskRegex)
{
	std::regex r(taskRegex);
	const auto v = tracker.listTasks();
	std::vector<std::string> matchingTasks;
	std::for_each(v.begin(), v.end(), [&](auto& e) {
		if (std::regex_match(e, r)) {
			matchingTasks.push_back(e);
		}
	});
	return matchingTasks;
}

std::vector<std::pair<Timestamp, Duration>> timeEachPeriod(const TimeTracker& tracker, const char* task, Timestamp start, Timestamp end, Duration period)
{
	const auto sessions = tracker.sessionsBetween(task, start, end);
	std::vector<std::pair<Timestamp, Duration>> times;
	Timestamp prevEnd = start;
	Timestamp endOfPeriod = start + period;
	size_t lastSessionIndex = 0;
	while(prevEnd < end && lastSessionIndex < sessions.size()) {		
		if (sessionInPeriod(sessions[lastSessionIndex], prevEnd, std::min(endOfPeriod, end))) {
			auto duration = Duration::zero();
			for (; lastSessionIndex < sessions.size(); ++lastSessionIndex) {
				const auto* ses = &sessions[lastSessionIndex];
				duration += chunkOfSession(*ses, prevEnd, std::min(end, endOfPeriod));
				if (ses->start + ses->duration > endOfPeriod) break;
			}
			times.emplace_back(prevEnd, duration);
		}		
		prevEnd = endOfPeriod;
		endOfPeriod += period;
	}
	return times;
}

Duration elapsedTime(const TimeTracker& tracker, const char* task, Timestamp start, Timestamp end)
{
	const auto v = tracker.sessionsBetween(task, start, end);
	Duration d = Duration::zero();
	std::for_each(v.begin(), v.end(), [&d](const auto& ses) {
		d += ses.duration;
	});
	return d;
}
