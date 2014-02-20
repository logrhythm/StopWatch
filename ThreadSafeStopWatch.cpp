#include "ThreadSafeStopWatch.h"

ThreadSafeStopWatch::ThreadSafeStopWatch() {
}  

StopWatch& ThreadSafeStopWatch::GetStopWatch()
{
   pthread_t tid { pthread_self() };

   const auto it = mStopWatchMap.find(tid);
   if (it == mStopWatchMap.end()) {
      std::lock_guard<std::mutex> lock(mMutex);
      // Lazy barbers lurk here, this should be safe
      const auto findIt = mStopWatchMap.find(tid);
      if (findIt == mStopWatchMap.end()) {
         return mStopWatchMap[tid];
      }
   }
   return it->second;
}
