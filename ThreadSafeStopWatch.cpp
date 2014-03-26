#include "ThreadSafeStopWatch.h"

ThreadSafeStopWatch::ThreadSafeStopWatch() {
}  

StopWatch& ThreadSafeStopWatch::GetStopWatch()
{
   pthread_t tid { pthread_self() };

   auto it = mStopWatchMap.find(tid);
   if (it == mStopWatchMap.end()) {
      std::lock_guard<std::mutex> lock(mMutex);
      // Lazy barbers lurk here, this should be safe
      it = mStopWatchMap.find(tid);
      if (it == mStopWatchMap.end()) {
         return mStopWatchMap[tid];
      }
   }
   return it->second;
}
