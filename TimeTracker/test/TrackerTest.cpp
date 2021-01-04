#include <gtest/gtest.h>
#define HIGH_RES_TESTING
#include <TrackerApp.h>
#include <thread>
using namespace std::chrono_literals;
std::unique_ptr<TimeTracker> ctx;
TEST(TrackerTest, durationTest)
{
	auto d = Duration::zero();
	auto lastStart = now();
	for (int i = 0; i < 10000; ++i)
	{
		if (rand() % 500 <= 1) {
			lastStart = now();
			d = Duration::zero();
		}
		const auto start = ctx->startTask("test");
		std::this_thread::sleep_for(std::chrono::nanoseconds(rand() % 1000));
		const auto end = ctx->pauseTask("test");
		d += end - start;
		if (rand() % 20 <= 1) {
			ASSERT_EQ(elapsedTime(*ctx, "test", lastStart, now()), d);
		}
	}
}
TEST(TrackerTest, durationPeriodLong)
{
	const auto period = std::chrono::duration_cast<Duration>(100us);
	for (int i = 0; i < 500; ++i) {
		const auto start = ctx->startTask("long");
		std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 10 + 1));
		const auto end = ctx->pauseTask("long");
		const auto times = timeEachPeriod(*ctx, "long", start, now(), period);
		Duration sum = Duration::zero();
		for (auto& t : times) {
			sum += t.second;
		}
		ASSERT_EQ(sum, end - start);
		ASSERT_EQ(times.size(), static_cast<size_t>(ceil(static_cast<double>((end - start).count()) /
			period.count())));
	}

}
TEST(TrackerTest, durationPeriods)
{
	auto d = Duration::zero();
	auto sum = Duration::zero();
	auto start = now();
	const auto period = std::chrono::duration_cast<Duration>(1000ns);
	for (int i = 0; i < 500; ++i)
	{
		d = Duration::zero();
		sum = Duration::zero();
		start = now();
		for (auto j = 0; j < 100; ++j) {
			const auto s = ctx->startTask("short");
			std::this_thread::sleep_for(std::chrono::nanoseconds(rand() % 300));
			const auto e = ctx->pauseTask("short");
			d += e - s;
			std::this_thread::sleep_for(std::chrono::nanoseconds(rand() % 100));
		}
		const auto times = timeEachPeriod(*ctx, "short", start, now(), period);
		const auto* prev = &times[0];
		sum += prev->second;
		for (size_t j = 1; j < times.size(); ++j) {
			ASSERT_GE(times[j].first - prev->first, period);
			ASSERT_EQ((times[j].first - prev->first) % period, 0ns);
			sum += times[j].second;
			prev = &times[j];
		}
		ASSERT_EQ(d, sum);
	}
}
int main(int argc, char** argv)
{
	ctx = getTracker(TrackerType::test);
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}