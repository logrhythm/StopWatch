/* 
 * File:   AlarmClock.h
 * Author: Amanda Carbonari
 * Created: January 5, 2016 4:35pm
 * Description: Utilizes the boost library to create a singleton interruptable thread
 *   that "communicates" with the main thread via a mutex, locks, and condition variables.
 */

#pragma once
#include <chrono>
#include <boost/thread.hpp>
#include <atomic>
#include <functional>

template<typename Duration> class AlarmClock {
public:
   typedef std::chrono::microseconds microseconds;
   typedef std::chrono::milliseconds milliseconds;
   typedef std::chrono::seconds seconds;

   AlarmClock(unsigned int sleepDuration, std::function<unsigned int (unsigned int)> funcPtr = nullptr) : mExpired(0),
      mSleptTime(0),
      mExit(false),
      mReset(false),
      kSleepTime(sleepDuration),
      kSleepTimeMsCount(ConvertToMillisecondsCount(Duration(sleepDuration))),
      kSleepTimeUsCount(ConvertToMicrosecondsCount(Duration(sleepDuration))),
      mSleepFunction(funcPtr) {
         if (mSleepFunction == nullptr) {
            mSleepFunction = std::bind(&AlarmClock::SleepUs, this, std::placeholders::_1);
         }
         mTimerThread = boost::thread(&AlarmClock::AlarmClockInterruptableThread, this);
   }

   virtual ~AlarmClock() {
      mExit.store(true);
      StopBackgroundThread();
   }
   
   bool Expired() {
      return mExpired.load();
   }

   void Reset() {
      boost::unique_lock<boost::mutex> lck(mMutex);
      mReset.store(true);

      // If the thread isn't expired, stop it.
      if (!mExpired.load()) {
         StopBackgroundThread();
      }

      // Reset the expired value and notify the thread to restart
      mExpired.store(0);
      {mCondition.notify_all();} // Needed in the case it is already waiting
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
         // Call the sleep function
         unsigned int retVal = mSleepFunction(kSleepTimeUsCount);

         if (retVal == 0) {
            // Expired, should increment mExpired
            mExpired++;
         } else if (mExit) { // The thread was interrupted on a destructor
            break;
         }

         boost::unique_lock<boost::mutex> lck(mMutex);

         if (!mReset) { // If the thread shouldn't reset
            try {
               // Wait for the reset method to notify (reduces unnecessary 
               // sleeping)
               mCondition.wait(lck); 
            } catch (boost::thread_interrupted e) {} // In the rare case
            // that the thread is interrupted during waiting, it means
            // that it should either exit (taken care of by the while) or
            // reset, which it will do automatically.
         }
         mReset.store(false);
      } while (!mExit);
   }
  
   void StopBackgroundThread() {
      mTimerThread.interrupt();
      // Check to see if the thread is joinable and only join if it is supposed
      // to exit.
      if (mTimerThread.joinable() && mExit) {
         mTimerThread.join();
      }   
   }

   unsigned int SleepUs(unsigned int t) {
      try {
         // Time points used in calculating the total overhead of the program.
         // They can be removed for production code or put in if statements. 
         // I wasn't sure which was a better idea.
         std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
         boost::this_thread::sleep_for(boost::chrono::microseconds(t));
         std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
         auto sleep_time = std::chrono::duration_cast<microseconds>(t2-t1).count();
         mSleptTime.store(sleep_time);
         return 0;
      } catch (boost::thread_interrupted e) {
         return 1;
      }
   }
   
   unsigned int ConvertToMillisecondsCount(Duration t) {
      return std::chrono::duration_cast<milliseconds>(t).count();
   }
   
   unsigned int ConvertToMicrosecondsCount(Duration t) {
      return std::chrono::duration_cast<microseconds>(t).count();
   }
   
   microseconds ConvertToMicroseconds(Duration t) {
      return boost::chrono::duration_cast<microseconds>(t);
   }
   
private:

   std::atomic<unsigned int> mExpired;
   std::atomic<unsigned int> mSleptTime;
   std::atomic<bool> mExit;
   std::atomic<bool> mReset;
   const int kSleepTime;
   const unsigned int kSleepTimeMsCount;
   const unsigned int kSleepTimeUsCount;
   std::function<unsigned int (unsigned int)> mSleepFunction;
   boost::thread mTimerThread;
   boost::mutex mMutex;
   boost::condition_variable mCondition;
};
