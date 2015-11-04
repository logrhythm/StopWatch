/* 
 * File:   MockAlarmClock.h
 * Author: Craig Cogdill
 * Created: November 3, 2015 2pm
 */

#pragma once
#include <iostream>
#include <atomic>


template<typename Duration> class MockAlarmClock {
public:   
   typedef std::chrono::microseconds microseconds;
   typedef std::chrono::milliseconds milliseconds;
   typedef std::chrono::seconds seconds;
   
   MockAlarmClock(unsigned int sleepDuration) : 
      mThreadAlive(false),
      mExpired(false),
      kSleepTime(sleepDuration),
      kSleepTimeMs(ConvertToMilliseconds(Duration(kSleepTime))),
      kSleepTimeUs(ConvertToMicroseconds(Duration(kSleepTime))),
      kNumberOfSleepIntervals(GetNumberOfSleepIntervals()),
      mExited(std::async(std::launch::async,
                         &MockAlarmClock::MockAlarmClockThread,
                         this)) {}
   
   virtual ~MockAlarmClock() {
      StopBackgroundThread();
   }
   
   bool Expired() {
      return mExpired.load();
   }

   void Reset() {
      if (!mExpired.load()) {
         StopBackgroundThread();
      }
      kNumberOfSleepIntervals = 0;
      mExpired.store(false); 
      mExited = std::async(std::launch::async, &MockAlarmClock::MockAlarmClockThread, this);
   }

   int SleepTimeUs() {
      return kSleepTimeUs;
   }
   
   int SleepTimeMs() {
      return kSleepTimeMs;
   }

   size_t GetNumberOfSleeps() {
      return kNumberOfSleepIntervals;
   }

   bool ThreadIsAlive() {
      return mThreadAlive.load();
   }

protected:

   void MockAlarmClockThread() {
      mThreadAlive.store(true);
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
      mExpired.store(true);
   }

   void Sleep(unsigned int sleepTime) {}

   void Sleep(microseconds t) {}

   void Sleep(milliseconds t) {}
   
   void Sleep(seconds t) {}

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
      size_t sleepIntervals = (kSleepTimeMs % kSmallestIntervalInMS == 0) ?
             ((kSleepTimeMs/kSmallestIntervalInMS) - 1) :
             (kSleepTimeMs/kSmallestIntervalInMS);
      kNumberOfSleepIntervals = sleepIntervals;
      return sleepIntervals;
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

   const int kSmallestIntervalInMS = 500;
   std::atomic<bool> mThreadAlive;
   std::atomic<bool> mExpired;
   const int kSleepTime;
   const int kSleepTimeMs;
   const int kSleepTimeUs;
   size_t kNumberOfSleepIntervals = 0;
   std::future<void> mExited;
};