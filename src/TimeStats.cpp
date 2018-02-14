#include "TimeStats.h"
#include <limits>
#include <algorithm>

TimeStats::TimeStats() :
   mMaxTime(0),
   mMinTime(std::numeric_limits<long long>::max()),
   mCount(0),
   mTotalTime(0) {}


void TimeStats::Save(long long ns) {
   ++mCount;
   mTotalTime += ns;
   mMaxTime = std::max(mMaxTime, ns);
   mMinTime = std::min(mMinTime, ns);
}

size_t TimeStats::ElapsedSec() {
   return mStopWatch.ElapsedSec();
}
void TimeStats::Flush(std::string& str) {
   str = {"Count: "};
   str += std::to_string(mCount)
          + ", Min time: " + std::to_string(mMinTime)  + " ns"
          + ", Max time: " + std::to_string(mMaxTime) + " ns : "
          + std::to_string(mMaxTime / 1000) + " us" +
          ", Average: " + std::to_string(GetAverage()) + " ns : " + std::to_string(GetAverage() / 1000) + " us";
   Reset();
}


TimeStats::Metrics TimeStats::Flush() {
   auto result = std::make_tuple(mMinTime, mMaxTime, mCount, mTotalTime, GetAverage());
   Reset();
   return result;
}


long long TimeStats::GetAverage() {
   if (mCount == 0) {
      return 0;
   }
   return mTotalTime / mCount;
}

void TimeStats::Reset() {
   mCount = mMaxTime = mTotalTime = 0;
   mMinTime = std::numeric_limits<long long>::max();
   mStopWatch.Restart();
}
