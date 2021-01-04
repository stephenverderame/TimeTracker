#include <gtest/gtest.h>
#define HIGH_RES_TESTING
#include <StreamLogger.h>
#include <functional>
#include <thread>
#include <StreamProvider.h>
#include <random>
using namespace std::chrono_literals;
using namespace maybe;
std::unique_ptr<StreamLogger> fl;
int testCount = 3000; //scales with the number of test iterations to run per test case
/**
 * Initializes the logger with random data
 * @param task the name of the task to initialize
 * @param amountOfRecords how many records the logger should have
 * @param f a predicate, if any, that will be called on each iteration and passed the number of records, and sessions
 * @return a list of sessions created in the logger
 */
std::vector<Session> initializeData(const char * task, std::unique_ptr<StreamLogger>& logger, size_t amountOfRecords, 
	std::function<void(size_t, size_t)> f = nullptr)
{
	logger->clearTask(task);
	bool start = false;
	std::vector<Session> sessions;
	Session activeSession;
	size_t recordCount = 0;
	for (size_t i = 0; i < amountOfRecords || (start && i >= amountOfRecords); ++i) //prevent incomplete records
	{
		if (rand() % 2) {
			const auto cur = logger->logBegin(task);
			if (!start) {
				++recordCount;
				activeSession.start = cur;
			}
			start = true;
		}
		else {
			const auto cur = logger->logEnd(task);
			if (start) {
				activeSession.duration = cur - activeSession.start;
				sessions.push_back(activeSession);
				++recordCount;
			}
			start = false;
		}
		if (f != nullptr) f(recordCount, sessions.size());
	}
	return sessions;
}
TEST(StreamTest, recordCount)
{
	auto startTime = now();
	initializeData("test", fl, testCount, [](size_t records, size_t) {
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
TEST(StreamTest, sessionAt)
{
	std::vector<Timestamp> invalidTimes;
	const auto sessions = 
	initializeData("session", fl, testCount, [&invalidTimes](auto records, auto) {
		if (records % 2 == 0 && (invalidTimes.empty() || rand() % invalidTimes.size() < 50)) {
			invalidTimes.push_back(now());
			std::this_thread::sleep_for(1ms);
		}
	});

	ASSERT_GT(invalidTimes.size(), 0);
	std::mt19937_64 random;
	random.seed(now().time_since_epoch().count());
	for (int i = 0; i < testCount; ++i)
	{
		if (rand() % 2) {
			const auto session = sessions[rand() % sessions.size()];
			const auto time = Duration(random() % session.duration.count()) + 
				session.start;
			const auto s = fl->sessionAt(time, "session");
			ASSERT_TRUE(isPresent(s));
			const auto& ses = getOrThrow(s);
			ASSERT_GE(time, ses.start);
			ASSERT_LE(time, ses.start + ses.duration);
		}
		else {
			const auto time = invalidTimes[i % invalidTimes.size()];
			const auto s = fl->sessionAt(time, "session");
			ASSERT_FALSE(isPresent(s));

		}
	}
}
TEST(StreamTest, sessionCount)
{
	const auto start = now();
	for (int i = 0; i < testCount; ++i) {
		const auto sessionCount = initializeData("sessionCount", fl, testCount / 5).size();
		ASSERT_EQ(sessionCount, fl->timeSpentBetween(start, now(), "sessionCount").size());
	}

}
TEST(StreamTest, sessionCount2)
{
	for (int i = 0; i < testCount / 3; ++i) {
		bool pickTime = false;
		auto start = now();
		const auto totalSessions = 
		initializeData("sessionCount", fl, testCount / 6, [&](auto, auto) {
			if (rand() % 500 == 1 && !pickTime) {
				start = now();
				pickTime = true;
				std::this_thread::sleep_for(1ms);
			}
		});	
		const auto testSessions = fl->timeSpentBetween(start, now(), "sessionCount");
		for (auto& e : testSessions) {
			ASSERT_GE(e.start, start);
		}
		for (auto& s : totalSessions) {
			ASSERT_EQ(std::find(testSessions.begin(), testSessions.end(), s) == 
				testSessions.end(), s.start < start);
		}
	}
}
TEST(StreamTest, existingStreams)
{
	char badChars[] = { ' ', '|', '$', '\\', '/', '"', '%' };
	for (int i = 0; i < testCount / 10; ++i)
	{
		std::vector<std::string> names;
		for (int j = 0; j < testCount / 100; ++j)
		{
			names.emplace_back(std::to_string(rand()) + badChars[j % sizeof(badChars)]);
			fl->logBegin(names.back().c_str());
		}
		const auto tks = fl->listAllTasks();
		for (auto& name : names) {
			ASSERT_NE(std::find(tks.begin(), tks.end(), name), tks.end());
			fl->clearTask(name.c_str());
		}
	}
}
int main(int argc, char ** argv)
{
	fl = std::make_unique<StreamLogger>(makeStreamProvider(StreamType::memory, "tests"));
	testing::InitGoogleTest(&argc, argv);
	const int first = RUN_ALL_TESTS();
	//File system logic is very similar, run tests again but not as thoroughly bc they're slower 
	//and most logic is repeated
	fl = std::make_unique<StreamLogger>(makeStreamProvider(StreamType::file, "tests"));
	testCount = 300;
	return RUN_ALL_TESTS() | first;
}