#pragma once

#include "TimeStats.h"
#include "StopWatch.h"



/**
Example usage:
TriggerTimeStats is a helper for TimeStats
It is a scope triggered "save" that makes it easy to use without code bloat.
The TriggerTimeStats and TimeStats adds overhead so it should only be used
when comparing relative speed of one operation vs another. It should not be
used in production code

TimeStats stats;
while(thread_loop) {
   ...
   { // scope around a function call that we want to instrument
      TriggerTimeStats trigger(stats);
      func();
   } // scope exits and `func` is measured.
}
*/
class TriggerTimeStats {
 public:
   TriggerTimeStats(TimeStats& timeStats)
      : mTimeStats(timeStats)
      , mSkip(false) {
      mStopWatch.Restart();
   }

   ~TriggerTimeStats() {
      if (!mSkip) {
         mTimeStats.Save(mStopWatch.ElapsedNs());

      }
   }

   void Skip() {
      mSkip = true;
   }


 private:
   TimeStats& mTimeStats;
   StopWatch mStopWatch;
   bool mSkip;
};