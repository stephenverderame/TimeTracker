#pragma once
#include "LoggerGateway.h"
#include "StreamProvider.h"
class StreamLogger : public LoggerGateway {
private:
	std::shared_ptr<StreamProvider> streams;
public:
	Timestamp logBegin(const char* task) override;
	Timestamp logEnd(const char* task) override;
	std::vector<Session> timeSpentBetween(const Timestamp& start, const Timestamp& end,
		const char* task) const override;

	///@return the number of log calls (logBegin + logEnd)
	size_t numLogs(const char * task) const;

	void clearTask(const char* task) override;
	bool doesTaskExist(const char* task) const override;

	const maybe::Maybe<Session> sessionAt(const Timestamp& time, const char* task) const override;

	std::vector<std::string> listAllTasks() const override;

	StreamLogger(std::shared_ptr<StreamProvider>&& provider) : streams(provider) {}
};