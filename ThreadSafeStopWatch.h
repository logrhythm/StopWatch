#pragma once
#include "StopWatch.h"
#include <boost/thread/mutex.hpp>
#include <unordered_set>
#include <unordered_map>

class ThreadSafeStopWatch {
public:
   typedef std::map<pthread_t, StopWatch> ThreadSafeStopWatchMap;

   ThreadSafeStopWatch();
   StopWatch &GetStopWatch();
private:
   boost::mutex mMutex;
   ThreadSafeStopWatchMap mStopWatchMap;
};
