#pragma once
#include <chrono>

// From: kjellkod.wordpress.com/2012/02/06/exploring-c11-part1-time/

class StopWatch {
public:
   typedef std::chrono::steady_clock clock;
   typedef std::chrono::microseconds microseconds;
   typedef std::chrono::milliseconds milliseconds;
   typedef std::chrono::seconds seconds;

   StopWatch();
   StopWatch(const StopWatch&) = delete;
   StopWatch& operator=(const StopWatch&) = delete;

   uint64_t ElapsedUs() const;
   uint64_t ElapsedMs() const;
   uint64_t ElapsedSec() const;

   std::chrono::steady_clock::time_point Restart();

private:
   clock::time_point mStart;
};

