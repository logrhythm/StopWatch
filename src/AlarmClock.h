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
   AlarmClock(unsigned int sleepDuration) : mExpired(false),
      kSleepTime(sleepDuration),
      kSleepTimeMs(ConvertToMilliseconds(Duration(kSleepTime))),
      kSleepTimeUs(ConvertToMicroseconds(Duration(kSleepTime))),
      mKeepRunning(new std::atomic<bool>(true)),
      mExited(std::async(std::launch::async,
                           &AlarmClock::AlarmClockThread,
                           this,
                           mKeepRunning)) {
   }
   
   virtual ~AlarmClock() {
      mKeepRunning->store(false);
      mExited.wait();
 }
   
   bool has_expired() {
      return mExpired.load();
   }

protected:

   void AlarmClockThread(std::shared_ptr<std::atomic<bool>> keepRunning) {
      if (Duration(kSleepTime) <= milliseconds(kSmallestIntervalInMS)) {
         Sleep(kSleepTime);
         mExpired.store(true);
      } else {
         AlarmClock::SleepInIntervals();
      }
   }
   
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
      size_t numberOfSleeps = GetNumberOfSleepIntervals();
      while (KeepRunning() && numberOfSleeps > 0) {
         Sleep(milliseconds(500));
         --numberOfSleeps; 
      }
      auto currentSleptFor = timer.ElapsedUs();
      auto keepRunning = KeepRunning();
      if (!keepRunning) {
         mExpired.store(true);
         return;
      }
      SleepForRemainder(currentSleptFor, keepRunning);
      mExpired.store(true);
   }

   bool KeepRunning() {
      return !mExpired.load() && mKeepRunning->load();
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
   std::shared_ptr<std::atomic<bool>> mKeepRunning;
   std::future<void> mExited;
};


