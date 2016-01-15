/* 
 * File:   AlarmClock.h
 * Author: Amanda Carbonari
 * Created: January 5, 2016 4:35pm
 * Description: Uses an "interruptable" std::thread, that checks every
 *    microsecond to see if any of the two interrupt atomics have been set to
 *    true (mReset, mExit). If they are it will stop it's sleep and then check
 *    to see if it was a reset or exit. If it was neither (expired) it will busy
 *    wait until mReset is set to true and it can start sleeping again.  
 */

#pragma once
#include <chrono>
#include <thread>
#include <atomic>
#include <functional>
#include "StopWatch.h"
#include <cassert>
using namespace std;

template<typename Duration> class AlarmClock {
public:
   typedef chrono::microseconds microseconds;
   typedef chrono::milliseconds milliseconds;
   typedef chrono::seconds seconds;

   // The sleep function is passed in for the unit tests. 
   AlarmClock(unsigned int sleepDuration, function<void (unsigned int)> funcPtr = nullptr) : mExpired(false),
      mSleptTime(0),
      mExit(false),
      mReset(false),
      mConstructorThreadId(this_thread::get_id()),
      kSleepTime(sleepDuration),
      kSleepTimeMsCount(ConvertToMillisecondsCount(Duration(sleepDuration))),
      kSleepTimeUsCount(ConvertToMicrosecondsCount(Duration(sleepDuration))),
      mSleepFunction(funcPtr) {
         if (mSleepFunction == nullptr) {
            mSleepFunction = [&](unsigned int sleepTime) 
            { 
               SleepUs(sleepTime); 
            }; 
         }
         mTimerThread = thread(&AlarmClock::AlarmClockInterruptableThread, this);
   }

   virtual ~AlarmClock() {
      mExit.store(true);
      StopBackgroundThread();
   }
   
   bool Expired() {
      return mExpired.load();
   }

   thread::id GetThreadId() {
      return mConstructorThreadId;
   }

   void Reset() {
      assert(mConstructorThreadId == GetThreadID() && "Illegal to call 'Reset' from thread other than originator");
      mReset.store(true);
      mExpired.store(false);
   }

   int SleepTimeUs() {
      return kSleepTimeUsCount;
   }
   
   int SleepTimeMs() {
      return kSleepTimeMsCount;
   }

protected:

   void AlarmClockInterruptableThread() {
      do {
         mSleepFunction(kSleepTimeUsCount);
         // This is required because the unit tests use their own "fake sleep"
         // pass it into the constructor. By doing that, the unit tests will
         // fail if the mExpired.store(true) is in the sleep function instead
         // of the thread function. aka Due to unit test limitation.
         if (!mExit && !mReset) {
            mExpired.store(true);
         }
         if (mExit) {  
            break;
         }

         if (mExpired) {
            while (!mReset && !mExit) {
               this_thread::sleep_for(chrono::microseconds(1));
            }
         }
         mReset.store(false);
      } while (!mExit);
   }
  
   void StopBackgroundThread() {
      if (mTimerThread.joinable() && mExit) {
         mTimerThread.join();
      }
   }

   void SleepUs(unsigned int t) {
      StopWatch sw;
      for (int i = 1; i < t; ++i) {
         this_thread::sleep_for(chrono::microseconds(1));
         auto elapsed = sw.ElapsedUs();
         if (mReset || mExit) {
            return;
         }
         if (elapsed >= t) {
            return;
         }
      }
   }
   
   unsigned int ConvertToMillisecondsCount(Duration t) {
      return chrono::duration_cast<milliseconds>(t).count();
   }
   
   unsigned int ConvertToMicrosecondsCount(Duration t) {
      return chrono::duration_cast<microseconds>(t).count();
   }
   
   microseconds ConvertToMicroseconds(Duration t) {
      return chrono::duration_cast<microseconds>(t);
   }
   
private:

   atomic<bool> mExpired;
   atomic<unsigned int> mSleptTime;
   atomic<bool> mExit;
   atomic<bool> mReset;
   const thread::id mConstructorThreadId;
   const int kSleepTime;
   const unsigned int kSleepTimeMsCount;
   const unsigned int kSleepTimeUsCount;
   function<void (unsigned int)> mSleepFunction;
   thread mTimerThread;
};
