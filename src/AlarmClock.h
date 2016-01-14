/* 
 * File:   AlarmClock.h
 * Author: Amanda Carbonari
 * Created: January 5, 2016 4:35pm
 * Description: Utilizes the boost library to create a singleton interruptable thread
 *   that "communicates" with the main thread via a mutex, locks, and condition variables.
 */

#pragma once
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <iostream>
#include "StopWatch.h"
#include <cassert>
using namespace std;

template<typename Duration> class AlarmClock {
public:
   typedef chrono::microseconds microseconds;
   typedef chrono::milliseconds milliseconds;
   typedef chrono::seconds seconds;

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
            mSleepFunction = bind(&AlarmClock::SleepUs, this, placeholders::_1);
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
         // Call the sleep function
         // cout << "THREAD: Calling sleep function" << endl;
         mSleepFunction(kSleepTimeUsCount);
         if (!mExit && !mReset) {
            mExpired.store(true);
         }
         // cout << "THREAD: atomic values, mExit = " << mExit << ", mExpired = " << mExpired << ", mReset = " << mReset << endl;
         if (mExit) { // The thread was interrupted on a destructor or 
            // the destructor was called during the sleep function
            // cout << "THREAD: mExit is " << mExit << " should exit." << endl;
            break;
         }

         if (mExpired) {
            // cout << "THREAD: mExpired is " << mExpired << " should wait for reset or exit." << endl;
            while (!mReset && !mExit) {
               this_thread::sleep_for(chrono::microseconds(1));
            }
            // cout << "THREAD: Finished waiting for reset or exit" << endl;
         }
         // cout << "THREAD: resetting reset value" << endl;
         mReset.store(false);
      } while (!mExit);
      // cout << "THREAD: done! exiting" << endl;
   }
  
   void StopBackgroundThread() {
      // Check to see if the thread is joinable and only join if it is supposed
      // to exit.
      if (mTimerThread.joinable() && mExit) {
         mTimerThread.join();
      }
   }

   void SleepUs(unsigned int t) {
      StopWatch sw;
      for (int i = 1; i < t; ++i) {
         this_thread::sleep_for(chrono::microseconds(1));
         if (mReset || mExit) {
            return;
         }
         if (sw.ElapsedUs() >= t) {
            mExpired.store(true);
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
   mutex mMutex;
   condition_variable mCondition;
};
