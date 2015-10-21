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
      std::cout << "Case 2.5 a: >= 1 second." <<std::endl;
      
         // How many sets of 500ms sleeps do we need
      size_t numberOfSleeps = ConvertToMilliseconds(Duration(kSleepTime)) / kSmallestIntervalInMS;
      std::cout << "number of sleeps = " << numberOfSleeps <<std::endl;

         // sleep for sets of 500 ms
      std::cout << " time to loop ======> " << timer.ElapsedUs() << std::endl;
      while (KeepRunning() && numberOfSleeps > 0) {
         std::cout << "sleeping for 500..." <<std::endl;
         Sleep(milliseconds(500));
         --numberOfSleeps; 
      }
      std::cout << " time through loop >>> " << timer.ElapsedUs() << std::endl;

         // figure out the last little bit
      auto currentSleptFor = sw.ElapsedUs();
      std::cout << "Been sleeping for " << currentSleptFor << " out of " << ConvertToMicroseconds(Duration(kSleepTime)) << std::endl;
      if (currentSleptFor < ConvertToMicroseconds(Duration(kSleepTime))) {
            // microseconds ms(ConvertToMicroseconds(Duration(kSleepTime)) - currentSleptFor);
            // Sleep(ms);
         Sleep(microseconds(ConvertToMicroseconds(Duration(kSleepTime)) - currentSleptFor));
      }
      
      auto currentSleptFor2 = sw.ElapsedUs();
      std::cout << "2nd time... Been sleeping for " << currentSleptFor2 << " out of " << ConvertToMicroseconds(Duration(kSleepTime)) << std::endl;
      std::cout << " time through function >>> " << timer.ElapsedUs() << std::endl;

      mExpired = true;
      std::cout << "It took " << sw.ElapsedUs() << " to get through SleepInIntervals." << std::endl;
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
   const int kSmallestIntervalInMS = 500;
};


