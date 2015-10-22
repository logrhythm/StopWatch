/* 
 * File:   AlarmClock.h
 * Author: Craig Cogdill
 * Created: October 15, 2015 10:45am
 */

#pragma once
#include <chrono>
#include <iostream>
#include <thread>
#include <atomic>
#include <czmq.h>
#include <future>
#include "StopWatch.h"

typedef std::chrono::microseconds microseconds;
typedef std::chrono::milliseconds milliseconds;
typedef std::chrono::seconds seconds;

template<typename Duration> class AlarmClock {
public:
   explicit AlarmClock(unsigned int sleepDuration) : AlarmClock(sleepDuration, false) {
   }

   AlarmClock(unsigned int sleepDuration, const int signalToSleepOn) : mExpired(false),
      kSleepTime(sleepDuration),
      kSleepTimeMs(ConvertToMilliseconds(Duration(kSleepTime))),
      kSleepTimeUs(ConvertToMicroseconds(Duration(kSleepTime))),
      kSleepUntilTrue(signalToSleepOn),
      mKeepRunning(new std::atomic<bool>(true)),
      mExited(std::async(std::launch::async,
                           &AlarmClock::AlarmClockThread,
                           this,
                           mKeepRunning)) {
   }
   
   virtual ~AlarmClock() {
      std::cout << "destructing" << std::endl;
      mKeepRunning->store(false);
      mExited.wait();
 }
   
   bool has_expired() {
      return mExpired.load();
   }

protected:

   void AlarmClockThread(std::shared_ptr<std::atomic<bool>> keepRunning) {
      std::cout << "We sleepin yo" << std::endl;
      if (Duration(kSleepTime) <= milliseconds(kSmallestIntervalInMS)) {
         std::cout << "Case 1: less than or equal to 500 milliseconds. Sleep for full time..." <<std::endl;
         Sleep(kSleepTime);
         std::cout << "All done sleepin" << std::endl;
         mExpired.store(true);
      } else {
         std::cout << "Case 2: greater than or equal to 500 milliseconds. Sleep in intervals..." <<std::endl;
         AlarmClock::SleepInIntervals();
      }
   }
   
   void Sleep(unsigned int sleepTime) {
      std::cout << "We sleepin for real this time" << std::endl;
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

   size_t GetNumberOfSleepIntervals() {
      return (kSleepTimeMs % kSmallestIntervalInMS == 0) ?
             ((kSleepTimeMs/kSmallestIntervalInMS) - 1) :
             (kSleepTimeMs/kSmallestIntervalInMS);
   }
  
   void SleepForRemainder(const unsigned int& currentSleptFor, const bool keepRunning) {
      if (currentSleptFor < kSleepTimeUs && keepRunning) {
         Sleep(microseconds(kSleepTimeUs - currentSleptFor));
      }
   } 

   void SleepInIntervals() {
      StopWatch timer;
     
      // How many sets of 500ms sleeps do we need
      size_t numberOfSleeps = GetNumberOfSleepIntervals();
      std::cout << "number of sleeps = " << numberOfSleeps <<std::endl;

      std::cout << "Given time divided by 500ms = " << kSleepTimeMs % kSmallestIntervalInMS << std::endl;
      // sleep for sets of 500 ms
      std::cout << "Value of kSleepUntilTrue = " << kSleepUntilTrue << std::endl;
      while (KeepRunning() && numberOfSleeps > 0) {
         Sleep(milliseconds(500));
         --numberOfSleeps; 
      }
      auto currentSleptFor = timer.ElapsedUs();

      auto keepRunning = KeepRunning();

      if (!keepRunning) {
         std::cout << "AlarmClock caught interrupt signal... exiting." << std::endl;
         mExpired.store(true);
         return;
      }

      SleepForRemainder(currentSleptFor, keepRunning);

      auto currentSleptFor2 = timer.ElapsedUs();
      std::cout << "2nd time... Been sleeping for " << currentSleptFor2 << " out of " << kSleepTimeUs << std::endl;

      mExpired.store(true);
   }

   bool KeepRunning() {
      return !mExpired.load() && mKeepRunning->load() && !kSleepUntilTrue;
   }
   
   unsigned int ConvertToMilliseconds(Duration t) {
      return std::chrono::duration_cast<milliseconds>(t).count();
   }
   
   unsigned int ConvertToMicroseconds(Duration t) {
      return std::chrono::duration_cast<microseconds>(t).count();
   }
   
private:

   std::atomic<bool> mExpired;
   const int kSleepTime;
   const int kSleepTimeMs;
   const int kSleepTimeUs;
   const int kSmallestIntervalInMS = 500;
   const int kSleepUntilTrue;
   std::shared_ptr<std::atomic<bool>> mKeepRunning;
   std::future<void> mExited;
};


