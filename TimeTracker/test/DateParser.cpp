#include <gtest/gtest.h>
#include <chrono>
#include <locale>
#include <ctime>
#include <AppTimes.h>
#include <thread>
/**
 * Class to show a message on destruction. Used for displaying information on assertation failure
 */
class MsgOnFail {
private:
	std::string s;
public:
	MsgOnFail(std::string&& msg) : s(std::move(msg)) {};
	void updateMsg(std::string&& msg) {
		s = std::move(msg);
	}
	~MsgOnFail() {
		if (!s.empty()) {
			try {
				printf("%s\n", s.c_str());
			}
			catch (std::exception&) {}
		}
	}
};


using namespace std::chrono_literals;
TEST(ParserTest, parseTime)
{
	const char* testFmt = "%F %H:%M:%S";
	auto lastTime = now();
	auto last = printTimestamp(lastTime, testFmt);
	std::cout << last << "\n";
	for (int i = 0; i < 1000; ++i)
	{
		lastTime += std::chrono::seconds(rand() % 10000 + 1);
		const auto res = printTimestamp(lastTime, testFmt);
		ASSERT_GT(res, last);
		ASSERT_LT(res, printTimestamp(lastTime + std::chrono::minutes(rand() % 10 + 1), testFmt));
		last = res;

	}
}
inline auto sign()
{
	return rand() % 2 ? -1 : 1;
}
inline auto randomTimeNearNow()
{
	return now() + (sign() * std::chrono::hours(rand() % 5000)) +
		(sign() * std::chrono::seconds(rand() % 10000)) +
		(sign() * std::chrono::minutes(rand() % 10000));
}
TEST(ParserTest, timestampTest)
{
	for (int i = 0; i < 1000; ++i)
	{
		auto time = randomTimeNearNow();
		auto s = printTimestamp(time);
		ASSERT_EQ(std::chrono::floor<std::chrono::seconds>(readTimestamp(s)), 
			std::chrono::floor<std::chrono::seconds>(time));
	}
}
const static auto fmts = std::vector<const char*>{
	"%Y-%m-%d",
	"%Y-%m-%d %H:%M:%S",
	"%m/%d/%Y %H:%M",
	"%m/%d/%y", "%m/%d/%y %H:%M:%S",
	"%b %d %Y",
	"%b %d/%Y",
	"%m/%d/%Y %I:%M:%S %p"
	//		"%d/%m/%y"
};
using days = std::chrono::duration<int, std::ratio<86400>>;
TEST(ParserTest, fmtGuessTest)
{
	MsgOnFail msg("");
	ASSERT_EQ(guessDateFormat("12/20/20"), "%m/%d/%y");
	for (int i = 0; i < 5000; ++i)
	{
		auto time = randomTimeNearNow();
		const char* fmt = fmts[rand() % fmts.size()];
		const auto str = printTimestamp(time, fmt);
		const auto guessedFmt = guessDateFormat(str);
		const auto parsedTime = readTimestamp(str, guessedFmt.c_str());
		msg.updateMsg(std::string("Failed on:") + fmt + std::string(" ") + str + " expected: " + guessedFmt + " on " + 
			printTimestamp(parsedTime, guessedFmt.c_str()));
		ASSERT_EQ(fmt, guessedFmt);		
	}
	msg.updateMsg("");
}

TEST(ParserTest, dateMatch)
{
	MsgOnFail msg("");
	for (int i = 0; i < 3000; ++i)
	{
		const auto fmt = fmts[i % fmts.size()];
		const auto time = randomTimeNearNow();
		const auto rdTime = readTimestamp(printTimestamp(time, fmt), fmt);
		std::cout << time.time_since_epoch().count() << "\n";
		std::cout << rdTime.time_since_epoch().count() << "\n";
		std::stringstream tm1;
		tm1 << " (" << std::chrono::floor<days>(time).time_since_epoch().count() << ") ";
		std::stringstream tm2;
		tm2 << " (" << std::chrono::floor<days>(rdTime).time_since_epoch().count() << ") ";
		msg.updateMsg(printTimestamp(time, fmt) + tm1.str() + " but read " + printTimestamp(rdTime, fmt) + tm2.str());
		ASSERT_EQ(printTimestamp(rdTime, fmt), printTimestamp(time, fmt));
//		ASSERT_EQ(std::chrono::time_point_cast<days>(rdTime), std::chrono::time_point_cast<days>(time));
	}
}
int main(int argc, char** argv)
{
	srand(clock());
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}