#include "TimeStatsTest.h"
#include "StopWatch.h"
#include "TimeStats.h"
#include <limits>
#include <thread>
#include <chrono>

#include "TriggerTimeStats.h"

namespace {
   const long long kNanoSecMinFake = 100;
   const long long kNanoSecMaxFake = 300;
   const long long kEmpty = 0;
   const long long kAverage = 200;
}

TEST_F(TimeStatsTest, StatsEmpty) {
   TimeStats stats;
   TimeStats::Metrics metrics = stats.FlushAsMetrics();
   EXPECT_EQ(std::numeric_limits<long long>::max(), std::get<TimeStats::Index::MinTime>(metrics));
   EXPECT_EQ(kEmpty, std::get<TimeStats::Index::MaxTime>(metrics));
   EXPECT_EQ(kEmpty, std::get<TimeStats::Index::Count>(metrics));
   EXPECT_EQ(kEmpty, std::get<TimeStats::Index::TotalTime>(metrics));
   EXPECT_EQ(0, std::get<TimeStats::Index::Average>(metrics));
}

TEST_F(TimeStatsTest, StatsEmpty2) {
   TimeStats stats;
   std::string metrics = stats.FlushAsString();
   std::string expected = "Count: 0, Min time: ";
   expected += std::to_string(std::numeric_limits<long long>::max());
   expected += " ns, Max time: 0 ns : 0 us,";
   expected += " Average: 0 ns : 0 us";
   EXPECT_EQ(metrics, expected);
}


TEST_F(TimeStatsTest, SimpleSetup) {
   TimeStats stats;
   stats.Save(kNanoSecMinFake);
   stats.Save(kNanoSecMaxFake);
   TimeStats::Metrics metrics = stats.FlushAsMetrics();
   EXPECT_EQ(kNanoSecMinFake, std::get<TimeStats::Index::MinTime>(metrics));
   EXPECT_EQ(kNanoSecMaxFake, std::get<TimeStats::Index::MaxTime>(metrics));
   EXPECT_EQ(2, std::get<TimeStats::Index::Count>(metrics));
   EXPECT_EQ(kNanoSecMinFake + kNanoSecMaxFake, std::get<TimeStats::Index::TotalTime>(metrics));
   EXPECT_EQ(kAverage, std::get<TimeStats::Index::Average>(metrics));
}

TEST_F(TimeStatsTest, SimpleSetup2) {
   TimeStats stats;
   long long kNanoSecMinFake = 100;
   long long kNanoSecMaxFake = 300;
   stats.Save(kNanoSecMinFake);
   stats.Save(kNanoSecMaxFake);
   std::string metrics = stats.FlushAsString();
   std::string expected = "Count: 2, Min time: 100";
   expected += " ns, Max time: 300 ns : 0 us,";
   expected += " Average: 200 ns : 0 us";
   EXPECT_EQ(metrics, expected);
}


TEST_F(TimeStatsTest, TimeTrigger1s) {
   TimeStats stats;
   using namespace std::chrono_literals;
   long long counter = 0;
   while (stats.ElapsedSec() <= 1) {
      ++counter;
      {
         TriggerTimeStats trigger{stats}; // implicit &
         std::this_thread::sleep_for(1ms);
      }
   }
   TimeStats::Metrics metrics = stats.FlushAsMetrics();
   long long expectedMinNs = 1 * 1000000;
   EXPECT_GT(std::get<TimeStats::Index::MinTime>(metrics), expectedMinNs);

   auto zeroMetrics = stats.FlushAsMetrics();
   EXPECT_EQ(std::numeric_limits<long long>::max(), std::get<TimeStats::Index::MinTime>(zeroMetrics));
   EXPECT_EQ(kEmpty, std::get<TimeStats::Index::MaxTime>(zeroMetrics));
   EXPECT_EQ(kEmpty, std::get<TimeStats::Index::Count>(zeroMetrics));
   EXPECT_EQ(kEmpty, std::get<TimeStats::Index::TotalTime>(zeroMetrics));
   EXPECT_EQ(0, std::get<TimeStats::Index::Average>(zeroMetrics));
}


