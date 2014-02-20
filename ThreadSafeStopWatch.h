#pragma once
#include "StopWatch.h"
#include <mutex>
#include <map>

class ThreadSafeStopWatch {
public:
   ThreadSafeStopWatch();
   StopWatch &GetStopWatch();
private:
   typedef std::map<pthread_t, StopWatch> ThreadSafeStopWatchMap;
   std::mutex mMutex;
   ThreadSafeStopWatchMap mStopWatchMap;
};
