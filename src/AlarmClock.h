/* 
 * File:   AlarmClock.h
 * Author: Craig Cogdill
 * Created: October 15, 2015 10:45am
 */

#pragma once
#include <chrono>
#include <thread>
#include <atomic>
#include <future>
#include "StopWatch.h"


template<typename Duration> class AlarmClock {
public:
   typedef std::chrono::microseconds microseconds;
   typedef std::chrono::milliseconds milliseconds;
   typedef std::chrono::seconds seconds;

   AlarmClock(unsigned int sleepDuration) : mExpired(false),
      kSleepTime(sleepDuration),
      kSleepTimeMs(ConvertToMilliseconds(Duration(kSleepTime))),
      kSleepTimeUs(ConvertToMicroseconds(Duration(kSleepTime))),
      mExited(std::async(std::launch::async,
                         &AlarmClock::AlarmClockThread,
                         this)) {
   }
   
   virtual ~AlarmClock() {
      StopBackgroundThread();
   }
   
   bool Expired() {
      return mExpired.load();
   }

   void Reset() {
      if (!mExpired.load()) {
         StopBackgroundThread();
      }
      mExpired.store(false); 
      mExited = std::async(std::launch::async, &AlarmClock::AlarmClockThread, this);
   }

   int SleepTimeUs() {
      return kSleepTimeUs;
   }
   
   int SleepTimeMs() {
      return kSleepTimeMs;
   }

protected:

   void AlarmClockThread() {
      SleepTimeIsBelow500ms() ? SleepForFullAmount() : SleepInIntervals();
   }
  
   void StopBackgroundThread() {
      mExpired.store(true);
      mExited.wait();
   }

   bool SleepTimeIsBelow500ms() {
      return Duration(kSleepTime) <= milliseconds(kSmallestIntervalInMS);
   }

   void SleepForFullAmount() {
      Sleep(kSleepTime);
      mExpired.store(true);
   }

   void Sleep(unsigned int sleepTime) {
      std::this_thread::sleep_for(Duration(sleepTime)); 
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

   // If 500ms is NOT an even divisor of the amount of sleep
   //    time given, should sleep for every multiple of 500ms
   //    that divides, then check the remaining time and just
   //    sleep for only that last bit.
   //
   // If 500ms IS an even divisor of the sleep time, sleep for 
   //    all but one multiple of 500ms. There is overhead induced
   //    by the while loop, so if the full amount of time is slept,
   //    the precision of the alarm clock will suffer.
   size_t GetNumberOfSleepIntervals() {
      return (kSleepTimeMs % kSmallestIntervalInMS == 0) ?
             ((kSleepTimeMs/kSmallestIntervalInMS) - 1) :
             (kSleepTimeMs/kSmallestIntervalInMS);
   }
  
   void SleepForRemainder(const unsigned int& currentSleptFor) {
      if (currentSleptFor < kSleepTimeUs) {
         Sleep(microseconds(kSleepTimeUs - currentSleptFor));
      }
   } 

   void SleepInIntervals() {
      std::cout << "AC: Sleep in intervals" << std::endl;
      StopWatch timer;
      size_t numberOfSleeps = GetNumberOfSleepIntervals();
      while (KeepRunning() && numberOfSleeps > 0) {
         Sleep(milliseconds(kSmallestIntervalInMS));
         --numberOfSleeps; 
      }
      auto currentSleptFor = timer.ElapsedUs();
      if (KeepRunning()) {
         SleepForRemainder(currentSleptFor);
      }
      mExpired.store(true);
   }

   bool KeepRunning() {
      return !mExpired.load();
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
   std::future<void> mExited;
   const int kSmallestIntervalInMS = 500;
};


