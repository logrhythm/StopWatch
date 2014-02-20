#include "ThreadSafeStopWatch.h"
#include "g2log.hpp"

ThreadSafeStopWatch::ThreadSafeStopWatch() {
   pthread_t tid;
   tid = pthread_self();
}  

StopWatch& ThreadSafeStopWatch::GetStopWatch()
{
   pthread_t tid;
   tid = pthread_self();

   const auto it = mStopWatchMap.find(tid);
   if (it == mStopWatchMap.end()) {
      boost::mutex::scoped_lock lock(mMutex);
      // Lazy barbers lurk here, this should be safe
      const auto findIt = mStopWatchMap.find(tid);
      if (findIt == mStopWatchMap.end()) {
         StopWatch stopWatch;
         mStopWatchMap[tid] = stopWatch;
         return mStopWatchMap[tid];
      }
   }
   return it->second;
}
