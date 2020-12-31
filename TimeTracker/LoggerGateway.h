#pragma once
#include <vector>
#include "AppTimes.h"
#include "Maybe.h"
class LoggerGateway {
public:
	virtual ~LoggerGateway() = default;
	virtual void logBegin(const char* task) = 0;
	virtual void logEnd(const char* task) = 0;
	/**
	 * @param start the earliest session start time
	 * @param task the name of the task
	 * @return a list of all sessions since start, in least to most rescent order
	 */
	virtual std::vector<Session> timeSpentSince(const Timestamp& start, const char* task) const = 0;

	/**
	 * Deletes all data for a particular task
	 */
	virtual void clearTask(const char* task) = 0;
	virtual bool doesTaskExist(const char* task) const = 0;

	virtual const maybe::Maybe<Session> sessionAt(const Timestamp& time, const char* task) const = 0;
};