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
    kSleepTime(sleepDuration) {
      StopWatch timer;
      if (Duration(kSleepTime) <= std::chrono::milliseconds(kSmallestIntervalInMS)) {
         Sleep(kSleepTime);
         mExpired = true;
      } else {
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

   void Sleep500MS() {
      std::this_thread::sleep_for(std::chrono::milliseconds(kSmallestIntervalInMS)); 
   }
   
   void SleepInIntervals(StopWatch& sw) {
      if (Duration(kSleepTime) >= std::chrono::seconds(1)) {
         std::cout << "Time is greater than one second. Split the timing up" <<std::endl;
         size_t numberOfSleeps = ConvertToMilliseconds(Duration(kSleepTime)) / kSmallestIntervalInMS;
         std::cout << "number of sleeps = " << numberOfSleeps <<std::endl;

         while (KeepRunning() && numberOfSleeps > 0) {
            std::cout << "sleeping for 500..." <<std::endl;
            Sleep500MS();
            --numberOfSleeps; 
         }
      } else {
         std::cout << "sleeping ..." <<std::endl;
         Sleep(kSleepTime);
      }
      mExpired = true;
      std::cout << "It took " << sw.ElapsedUs() << " to get through SleepInIntervals." << std::endl;
   }

   bool KeepRunning() {
      return !mExpired && !mExited;
   }
   
   unsigned int ConvertToMilliseconds(Duration t) {
      return std::chrono::duration_cast<milliseconds>(t).count();
   }
   
private:

   bool mExpired;
   bool mExited;
   const int kSleepTime;
   const int kSmallestIntervalInMS = 500;
};


