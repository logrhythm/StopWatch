/*
 * File:   StopWatch.h
 * Author: KjellKod
 * From: https://github.com/KjellKod/StopWatch
 *
 * Created on 2014-02-07
 */


#pragma once
#include <chrono>


/**
* Default argument is steady_clock this can be used as such
* StopWatch s; s.ElapsedUs();
* (i.e. this can also be used as
* ChronoMeter<> s; s.ElapsedUs(); )
*
* For higher definition measurements but with no 'steady clock' guarantees
* the high resolution clock can be used instead
* PrecisionStopWatch s; s.ElapsedNs();
* (i.e. this can alse be used as:
* ChronoMeter<std::chrono::high_resolution_clock> s; s.ElapsedUs(); )
*/
template<typename ChronoType = std::chrono::steady_clock> class ChronoMeter {
 public:
   typedef ChronoType clock;
   typedef std::chrono::nanoseconds nanoseconds;
   typedef std::chrono::microseconds microseconds;
   typedef std::chrono::milliseconds milliseconds;
   typedef std::chrono::seconds seconds;

   ChronoMeter() : mStart(clock::now()) {}
   ChronoMeter(const ChronoMeter& other): mStart(other.mStart) {}


   ChronoMeter& operator=(const ChronoMeter& rhs) {
      mStart = rhs.mStart;
      return *this;
   }

   /**
    * You might want to consider using this function from PrecisionStopWatch instead of 
    * the standard StopWatch. On the ns level the high_resolution_clock does make a 
    * difference.
    */
   uint64_t ElapsedNs() const {
      return std::chrono::duration_cast<nanoseconds>(clock::now() - mStart).count();
   }

   uint64_t ElapsedUs() const {
      return std::chrono::duration_cast<microseconds>(clock::now() - mStart).count();
   }

   uint64_t ElapsedMs() const {
      return std::chrono::duration_cast<milliseconds>(clock::now() - mStart).count();
   }

   uint64_t ElapsedSec() const {
      return std::chrono::duration_cast<seconds>(clock::now() - mStart).count();
   }


   /**
    * Resets the start point
    * @return the updated start point
    */
   typename ChronoType::time_point Restart() {
      mStart = clock::now();
      return mStart;
   }

 protected:
   typename clock::time_point mStart;
};

using StopWatch = ChronoMeter<>;
using PrecisionStopWatch = ChronoMeter<std::chrono::high_resolution_clock>;


