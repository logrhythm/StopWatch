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
      {
         // cout << "DESTRUCTOR: Obtaining lock" << endl;
         unique_lock<mutex> lck(mMutex, try_to_lock);
         // cout << "DESTRUCTOR: Setting mExit to true" << endl;
         mExit.store(true);
         // cout << "DESTRUCTOR: Notifying all" << endl;
         mCondition.notify_all();
      }
      // cout << "DESTRUCTOR: Stopping background thread" << endl;
      StopBackgroundThread();
      // cout << "DESTRUCTOR: Finished!" << endl;
   }
   
   bool Expired() {
      return mExpired.load();
   }

   void Reset() {
      // cout << "RESET: Creating lock" << endl;
      unique_lock<mutex> lck(mMutex, try_to_lock);
      // cout << "RESET: Setting mReset to true" << endl;
      mReset.store(true);
      // // If the thread isn't expired, stop it.
      // if (!mExpired.load()) {
      //    StopBackgroundThread();
      // }
      // Reset the expired value and notify the thread to restart
      // cout << "RESET: setting mExpired to 0" << endl;
      mExpired.store(0);
      // cout << "RESET: notifying all" << endl;
      mCondition.notify_all(); // Needed in the case it is already waiting
      // cout << "RESET: finished!" << endl;
   }

   // Used for performance testing, can be removed. 
   unsigned int SleptTime() {
      return mSleptTime.load();
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
         cout << "THREAD: calling sleep function" << endl;
         // Call the sleep function
         unsigned int retVal = mSleepFunction(kSleepTimeUsCount);

         if (retVal == 0) {
            cout << "THREAD: expired! " << (mExpired + 1) << endl;
            // Expired normally, should increment mExpired
            mExpired++;
         } 

         if (mExit) { // The thread was interrupted on a destructor or 
            // the destructor was called during the sleep function
            cout << "THREAD: break!" << endl;
            break;
         }

         while (!mReset || !mExit) { // If the thread shouldn't reset
            // cout << "THREAD: Grabbing lock" << endl;
            unique_lock<mutex> lck(mMutex, defer_lock);
            cout << "THREAD: Shouldn't reset, waiting on lock, " << lck.owns_lock() << endl;
            // Wait to get notified. It will get notified under two conditions:
            //    1) Should restart
            //    2) Should exit
            // If it should exit, the while portion of the do while will execute,
            // if it should restart, it will automatically loop. 
            {
               mCondition.wait(lck);
            } 
            cout << "THREAD: Done waiting on lock!" << endl;
            lck.unlock();
         }
         cout << "THREAD: setting reset to false because we are resetting" << endl;
         mReset.store(false);
         cout << "THREAD: checking while loop" << endl;
      } while (!mExit);
      // cout << "THREAD: exiting!" << endl;
   }
  
   void StopBackgroundThread() {
      // Change to setting the interrupt to atomic. It should then notify?
      // cout << "STOPPER: Checking if joinable and exit" << endl;
      // Check to see if the thread is joinable and only join if it is supposed
      // to exit.
      if (mTimerThread.joinable() && mExit) {
         // cout << "STOPPER: Notifying all threads" << endl;
         mCondition.notify_all();
         // cout << "STOPPER: joining with thread" << endl;
         mTimerThread.join();
      }
      // cout << "STOPPER: finished and exiting" << endl;
   }

   unsigned int SleepUs(unsigned int t) {
      cout << "SLEEPER: Starting for loop" << endl;
      chrono::high_resolution_clock::time_point start = chrono::high_resolution_clock::now();
      for (int i = 1; i < t; i++) {
         this_thread::sleep_for(chrono::microseconds(1));
         if (mReset || mExit) {
            cout << "SLEEPER: reset or exit" << endl;
            return 1;
         }
      }
      chrono::high_resolution_clock::time_point end = chrono::high_resolution_clock::now();
      mSleptTime = chrono::duration_cast<microseconds>(end - start).count();
      cout << "SLEEPER: expired" << endl;
      return 0;
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
