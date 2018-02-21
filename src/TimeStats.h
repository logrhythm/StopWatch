#pragma once
#include <string>
#include <tuple>
#include "StopWatch.h"

/**

Example usage:
TimeStats is a easy to use instrumentation tool for measuring how expensive
function calls are. A typical usage would be to measure many thousands of operations and get the max, min and average time to execute this.

See also "TriggerTimeStats" which reduced coat bloat.

TimeStats adds overhead so it should only be used when comparing relative speed of one operation vs another. It should not be used in production code

TimeStats stats;
while(thread_loop) {
   ...
   { // scope around a function call that we want to instrument
      TriggerTimeStats trigger(stats);
      func();
   } // scope exits and `func` is measured.


   if (stats.ElapsedSec() == X) {
      LOG(INFO) << "Expensive call metrics: " << stats.Flush();
   }
} // end loop
*/


class TimeStats {
 public:

   TimeStats();
   ~TimeStats() = default;

   void Save(long long ns);
   void Flush(std::string& str);
   enum Index { MinTime = 0,
                MaxTime = 1,
                Count = 2,
                TotalTime = 3, 
                Average = 4
              };

   using Metrics = std::tuple<long long, long long, long long, long long, long long>;
   Metrics Flush();
   size_t ElapsedSec();

 private:
   void Reset();
   long long GetAverage();

   long long mMaxTime;
   long long mMinTime;
   long long mCount;
   long long mTotalTime;
   StopWatch mStopWatch;


};