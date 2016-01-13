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
using namespace std;

template<typename Duration> class AlarmClock {
public:
   typedef chrono::microseconds microseconds;
   typedef chrono::milliseconds milliseconds;
   typedef chrono::seconds seconds;

   AlarmClock(unsigned int sleepDuration, function<unsigned int (unsigned int)> funcPtr = nullptr) : mExpired(0),
      mSleptTime(0),
      mExit(false),
      mReset(false),
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
      unique_lock<mutex> lck(mMutex);
      mExit.store(true);
      lck.unlock();
      mCondition.notify_all();
      StopBackgroundThread();
   }
   
   bool Expired() {
      return mExpired.load();
   }

   void Reset() {
      unique_lock<mutex> lck(mMutex);
      mReset.store(true);
      mExpired.store(0);
      lck.unlock();
      mCondition.notify_all(); // Needed in the case it is already waiting
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
         unsigned int retVal = mSleepFunction(kSleepTimeUsCount);

         if (retVal == 0) {
            // Expired normally, should increment mExpired
            mExpired++;
         } 

         if (mExit) { // The thread was interrupted on a destructor or 
            // the destructor was called during the sleep function
            break;
         }

         while (!mReset && !mExit) { // If the thread shouldn't reset or exit
            unique_lock<mutex> lck(mMutex);
            // Wait to get notified. It will get notified under two conditions:
            //    1) Should restart
            //    2) Should exit
            // If it should exit, the while portion of the do while will execute,
            // if it should restart, it will automatically loop. 
            {
               auto now = chrono::high_resolution_clock::now();
               auto microTime = ConvertToMicroseconds(Duration(kSleepTime));
               mCondition.wait_until(lck, now + microTime + microTime);
            } 
            lck.unlock();
         }
         mReset.store(false);
      } while (!mExit);
   }
  
   void StopBackgroundThread() {
      // Change to setting the interrupt to atomic. It should then notify?
      // Check to see if the thread is joinable and only join if it is supposed
      // to exit.
      if (mTimerThread.joinable() && mExit) {
         mCondition.notify_all();
         mTimerThread.join();
      }
   }

   unsigned int SleepUs(unsigned int t) {
      unsigned int val = -1;
      StopWatch sw;
      for (int i = 1; i < t; ++i) {
         this_thread::sleep_for(chrono::microseconds(1));
         if (mReset || mExit) {
            val = 1;
            break;
         }
         if (sw.ElapsedUs() >= t) {
               val = 0;
               break;
            }
      }
      return val;
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

   atomic<unsigned int> mExpired;
   atomic<unsigned int> mSleptTime;
   atomic<bool> mExit;
   atomic<bool> mReset;
   const int kSleepTime;
   const unsigned int kSleepTimeMsCount;
   const unsigned int kSleepTimeUsCount;
   function<unsigned int (unsigned int)> mSleepFunction;
   thread mTimerThread;
   mutex mMutex;
   condition_variable mCondition;
};
