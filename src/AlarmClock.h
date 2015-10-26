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
      mKeepRunning(new bool(true)),
      mExited(std::async(std::launch::async,
                         &AlarmClock::AlarmClockThread,
                         this,
                         mKeepRunning)) {
   }
   
   virtual ~AlarmClock() {
      mKeepRunning = false;
      mExited.wait();
   }
   
   bool Expired() {
      return mExpired.load();
   }

   int SleepTimeUs() {
      return kSleepTimeUs;
   }
   
   int SleepTimeMs() {
      return kSleepTimeMs;
   }

protected:

   void AlarmClockThread(std::shared_ptr<bool> keepRunning) {
      SleepTimeIsBelow500ms() ? AlarmClock::SleepForFullAmount() : AlarmClock::SleepInIntervals();
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
      StopWatch timer;
      size_t numberOfSleeps = GetNumberOfSleepIntervals();
      while (KeepRunning() && numberOfSleeps > 0) {
         Sleep(milliseconds(kSmallestIntervalInMS));
         --numberOfSleeps; 
      }
      auto currentSleptFor = timer.ElapsedUs();
      if (!KeepRunning()) {
         mExpired.store(true);
         return;
      }
      SleepForRemainder(currentSleptFor);
      mExpired.store(true);
   }

   bool KeepRunning() {
      return !mExpired.load() && mKeepRunning;
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
   std::shared_ptr<bool> mKeepRunning;
   std::future<void> mExited;
};


