/* 
 * File:   AlarmClock.h
 * Author: Craig Cogdill
 * Created: October 15, 2015 10:45am
 */

#pragma once
#include <chrono>
#include <iostream>
#include <thread>
#include <czmq.h>
#include "StopWatch.h"

typedef std::chrono::microseconds microseconds;
typedef std::chrono::milliseconds milliseconds;
typedef std::chrono::seconds seconds;

template<typename Duration> class AlarmClock {
public:
   explicit AlarmClock(unsigned int sleepDuration) : mExpired(false),
    mExited(false),
    kSleepTime(sleepDuration),
    kSleepTimeMs(ConvertToMilliseconds(Duration(kSleepTime))),
    kSleepTimeUs(ConvertToMicroseconds(Duration(kSleepTime))) {
      StopWatch timer;
      if (Duration(kSleepTime) <= milliseconds(kSmallestIntervalInMS)) {
         std::cout << "Case 1: less than or equal to 500 milliseconds. Sleep for full time..." <<std::endl;
         Sleep(kSleepTime);
         mExpired = true;
      } else {
         std::cout << "Case 2: greater than or equal to 500 milliseconds. Sleep in intervals..." <<std::endl;
         AlarmClock::SleepInIntervals(timer);
      }
   }
   
   virtual ~AlarmClock() {
      mExited = true;
   }
   
   bool has_expired() {
      return mExpired;
   }

   void SmartSleep() {
   }
   // AlarmClock& operator=(const AlarmClock& rhs);

protected:
   void Sleep(unsigned int sleepTime) {
      std::this_thread::sleep_for(Duration(kSleepTime)); 
   }

   void Sleep(microseconds t) {
      std::this_thread::sleep_for(t);
   }

   void Sleep(milliseconds t) {
      std::this_thread::sleep_for(t); 
   }
   
   void Sleep(seconds t) {
      std::this_thread::sleep_for(t); 
   }
   
   void SleepInIntervals(StopWatch& sw) {
      StopWatch timer;
     
      // How many sets of 500ms sleeps do we need
      size_t numberOfSleeps;

      std::cout << "Given time divided by 500ms = " << kSleepTimeMs % kSmallestIntervalInMS << std::endl;
      if (kSleepTimeMs % kSmallestIntervalInMS == 0) {
         numberOfSleeps = kSleepTimeMs / kSmallestIntervalInMS - 1;
         std::cout << "Divides exactly into 500ms" << std::endl;
      } else {
         numberOfSleeps = kSleepTimeMs / kSmallestIntervalInMS;
         std::cout << "Does not divide exactly into 500ms" <<  std::endl;
      }
      std::cout << "number of sleeps = " << numberOfSleeps <<std::endl;

      // sleep for sets of 500 ms
      while (KeepRunning() && numberOfSleeps > 0) {
         Sleep(milliseconds(500));
         --numberOfSleeps; 
      }
      auto currentSleptFor = sw.ElapsedUs();

      // figure out the last little bit
      std::cout << "Been sleeping for " << currentSleptFor << " out of " << kSleepTimeUs << std::endl;
      if (currentSleptFor < kSleepTimeUs) {
         Sleep(microseconds(kSleepTimeUs - currentSleptFor));
      }

      auto currentSleptFor2 = sw.ElapsedUs();
      std::cout << "2nd time... Been sleeping for " << currentSleptFor2 << " out of " << kSleepTimeUs << std::endl;

      mExpired = true;
   }

   bool KeepRunning() {
      return !mExpired && !mExited;
   }
   
   unsigned int ConvertToMilliseconds(Duration t) {
      return std::chrono::duration_cast<milliseconds>(t).count();
   }
   
   unsigned int ConvertToMicroseconds(Duration t) {
      return std::chrono::duration_cast<microseconds>(t).count();
   }
   
private:

   bool mExpired;
   bool mExited;
   const int kSleepTime;
   const int kSleepTimeMs;
   const int kSleepTimeUs;
   const int kSmallestIntervalInMS = 500;
};


