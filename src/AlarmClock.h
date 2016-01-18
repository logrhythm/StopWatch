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
using namespace std;

template<typename Duration> class AlarmClock {
public:
   typedef chrono::microseconds microseconds;

   // The sleep function is passed in for the unit tests. 
   AlarmClock(unsigned int sleepDuration, function<bool (unsigned int)> funcPtr = nullptr) : mExpired(false),
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
         mAlarmThread = thread(&AlarmClock::AlarmClockInterruptableThread, this);
      }

   virtual ~AlarmClock() {
      mExit.store(true, memory_order_relaxed);
      if (mAlarmThread.joinable()) {
         mAlarmThread.join();
      }
   }
   
   bool Expired() {
      return mExpired.load();
   }

   void Reset() {
      mReset.store(true, memory_order_relaxed);
      mExpired.store(false);
   }

   int SleepTimeUs() {
      return kSleepTimeUsCount;
   }

protected:

   void AlarmClockInterruptableThread() {
      while(!mExit) {
         if(mAlarmExpiredFunction(kSleepTimeUsCount)) {
            mExpired.store(true);
            while (!mReset && !mExit) {
               this_thread::sleep_for(microseconds(1));
            }
         }
         mReset.store(false, memory_order_relaxed);
      }
   }

   bool ExpireAtUs(unsigned int timeUsTillExpire) {
      StopWatch sw;

      // The loop introduces a 50 microsecond overhead because it accesses
      // the two atomics. Therefore stopwatch is needed to ensure the alarm
      // does not over sleep. 
      while (sw.ElapsedUs() < timeUsTillExpire) {
         this_thread::sleep_for(microseconds(1));
         if (mReset || mExit) {
            return false;
         }
      }

      return true;
   }
   
   unsigned int ConvertToMicrosecondsCount(Duration t) {
      return chrono::duration_cast<microseconds>(t).count();
   }
   
private:

   atomic<bool> mExpired;
   atomic<bool> mExit;
   atomic<bool> mReset;
   const unsigned int kSleepTimeUsCount;
   function<bool (unsigned int)> mAlarmExpiredFunction;
   thread mAlarmThread;
};
