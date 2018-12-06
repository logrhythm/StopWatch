/* 
 * File:   AlarmClock.h
 * Author: Amanda Carbonari
 * Created: January 5, 2016 4:35pm
 * Description: Uses an "interruptable" std::thread, that checks every
 *    microsecond to see if any of the two interrupt atomics have been set to
 *    true (mReset, mExit). If they are it will stop it's sleep and then check
 *    to see if it was a reset or exit. If it was expired and not reset or exit it will sleep
 *    until mReset (or exit) is set to true and it can start sleeping again.  
 */

#pragma once
#include <chrono>
#include <thread>
#include <atomic>
#include <functional>
#include <iostream>
#include "StopWatch.h"

template<typename Duration> class AlarmClock {
public:
   typedef std::chrono::microseconds microseconds;

   // The sleep function is passed in for the unit tests. 
   AlarmClock(unsigned int sleepDuration, std::function<bool (unsigned int)> funcPtr = nullptr) : mExpired(false),
      mExit(false),
      mReset(false),
      kSleepTimeUsCount(ConvertToMicrosecondsCount(Duration(sleepDuration))),
      mAlarmExpiredFunction(funcPtr) {
         if (mAlarmExpiredFunction == nullptr) {
            mAlarmExpiredFunction = [&](unsigned int sleepTime) -> bool
            { 
               return ExpireAtUs(sleepTime); 
            }; 
         }
         mAlarmThread = std::thread(&AlarmClock::AlarmClockInterruptableThread, this);
      }

   virtual ~AlarmClock() {
      mExit.store(true, std::memory_order_release);
      if (mAlarmThread.joinable()) {
         mAlarmThread.join();
      }
   }
   
   bool Expired() {
      return mExpired.load(std::memory_order_acquire);
   }

   void Reset() {
      mReset.store(true, std::memory_order_release);
      mExpired.store(false, std::memory_order_release);
   }

   int SleepTimeUs() {
      return kSleepTimeUsCount;
   }

protected:

   void AlarmClockInterruptableThread() {
      while(!mExit.load(std::memory_order_acquire)) {
         if(mAlarmExpiredFunction(kSleepTimeUsCount)) {
            mExpired.store(true, std::memory_order_release);
            while (!mReset.load(std::memory_order_acquire) && !mExit.load(std::memory_order_acquire)) {
               std::this_thread::sleep_for(microseconds(1));
            }
         }
         mReset.store(false, std::memory_order_release);
      }
   }

   bool ExpireAtUs(unsigned int timeUsTillExpire) {
      StopWatch sw;
      // The loop introduces a 50 microsecond overhead because it accesses
      // the two atomics. Therefore stopwatch is needed to ensure the alarm
      // does not over sleep. 
      while (sw.ElapsedUs() < timeUsTillExpire) {
         std::this_thread::sleep_for(microseconds(25));
         if (mReset.load(std::memory_order_acquire) || mExit.load(std::memory_order_acquire)) {
            return false;
         }
      }

      return true;
   }
   
   unsigned int ConvertToMicrosecondsCount(Duration t) {
      return std::chrono::duration_cast<microseconds>(t).count();
   }
   
private:

   std::atomic<bool> mExpired;
   std::atomic<bool> mExit;
   std::atomic<bool> mReset;
   const unsigned int kSleepTimeUsCount;
   std::function<bool (unsigned int)> mAlarmExpiredFunction;
   std::thread mAlarmThread;
};
