#pragma once
#include <vector>
#include "AppTimes.h"
#include "Maybe.h"
class LoggerGateway {
public:
	virtual ~LoggerGateway() = default;
	/// @return the timestamp of the new begin record
	virtual Timestamp logBegin(const char* task) = 0;
	/// @return the timestamp of the new end record
	virtual Timestamp logEnd(const char* task) = 0;
	/**
	 * @param start the earliest session start time
	 * @param end the latest session end time
	 * @param task the name of the task
	 * @return a list of all sessions since start, in least to most rescent order. 
	 * Sessions are completed within start - end
	 */
	virtual std::vector<Session> timeSpentBetween(const Timestamp& start, const Timestamp& end,
		const char* task) const = 0;

	/**
	 * Deletes all data for a particular task
	 */
	virtual void clearTask(const char* task) = 0;
	virtual bool doesTaskExist(const char* task) const = 0;

	virtual const maybe::Maybe<Session> sessionAt(const Timestamp& time, const char* task) const = 0;
};