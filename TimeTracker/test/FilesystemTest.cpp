#include <gtest/gtest.h>
#include "../StreamLogger.h"
#include <functional>
#include <thread>
#include "../MemoryStreamProvider.h"
using namespace std::chrono_literals;
using namespace maybe;
std::unique_ptr<StreamLogger> fl;
/**
 * Initializes the logger with random data
 * @param task the name of the task to initialize
 * @param amountOfRecords how many records the logger should have
 * @param f a predicate, if any, that will be called on each iteration and passed the number of records
 * @return the number of sessions created in the logger
 */
size_t initializeData(const char * task, std::unique_ptr<StreamLogger>& logger, size_t amountOfRecords, 
	std::function<void(size_t, size_t)> f = nullptr)
{
	logger->clearTask(task);
	bool start = false;
	size_t sessionCount = 0;
	size_t recordCount = 0;
	for (size_t i = 0; i < amountOfRecords; ++i)
	{
		if (rand() % 2) {
			logger->logBegin(task);
			if (!start) ++recordCount;
			start = true;
		}
		else {
			logger->logEnd(task);
			if (start) {
				++sessionCount;
				++recordCount;
			}
			start = false;
		}
		if (f != nullptr) f(recordCount, sessionCount);
	}
	return sessionCount;
}
TEST(FileSystemTest, recordCount)
{
	auto startTime = now();
	initializeData("test", fl, 3000, [](size_t records, size_t) {
		if (rand() % 5 == 1) {
			try {
				ASSERT_EQ(records, fl->numLogs("test"));
			}
			catch (std::exception&) {
				ASSERT_EQ(records, 0);
			}
		}
		});
	ASSERT_TRUE(fl->doesTaskExist("test"));
	
}
TEST(FileSystemTest, sessionAt)
{
	std::vector<Timestamp> validTimes;
	std::vector<Timestamp> invalidTimes;
	initializeData("session", fl, 3000, [&validTimes, &invalidTimes](auto records, auto) {
		if (records % 2 && (validTimes.empty() || rand() % validTimes.size() < 20)) {
			validTimes.push_back(now());
		}
		else if (records % 2 == 0 && (invalidTimes.empty() || rand() % invalidTimes.size() < 20)) {
			invalidTimes.push_back(now());
			std::this_thread::sleep_for(1ms);
		}
	});

	ASSERT_GT(validTimes.size(), 0);
	ASSERT_GT(invalidTimes.size(), 0);
	for (int i = 0; i < 1000; ++i)
	{
		if (rand() % 2) {
			const auto index = i % validTimes.size();
			const auto s = fl->sessionAt(validTimes[index], "session");
			ASSERT_TRUE(isPresent(s));
			const auto& ses = getOrThrow(s);
			ASSERT_GE(validTimes[index], ses.start);
			ASSERT_LE(validTimes[index], ses.start + ses.duration);
		}
		else {
			const auto time = invalidTimes[i % invalidTimes.size()];
			const auto s = fl->sessionAt(time, "session");
/*			if (isPresent(s)) {
				const auto& ses = getOrThrow(s);
				ASSERT_GE(time, ses.start);
				ASSERT_LE(time, ses.start + ses.duration);
			}*/
			ASSERT_FALSE(isPresent(s));

		}
	}
}
TEST(FileSystemTest, sessionCount)
{
	const auto start = now();
	for (int i = 0; i < 1000; ++i) {
		const auto sessions = initializeData("sessionCount", fl, 300);
		ASSERT_EQ(sessions, fl->timeSpentSince(start, "sessionCount").size());
	}

}
int main(int argc, char ** argv)
{
	fl = std::make_unique<StreamLogger>(std::make_shared<MemStreamProvider>());
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}