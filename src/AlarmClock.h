/* 
 * File:   AlarmClock.h
 * Author: Amanda Carbonari
 * Created: January 5, 2016 4:35pm
 * Description: Utilizes the boost library to create a singleton interruptable thread
 *   that "communicates" with the main thread via a mutex, locks and condition variables.
 * TODO: Must add test to see if the mutex is reentrant (multiple resets)
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
      mExit(false),
      kSleepTime(sleepDuration),
      kSleepTimeMsCount(ConvertToMillisecondsCount(Duration(sleepDuration))),
      kSleepTimeUsCount(ConvertToMicrosecondsCount(Duration(sleepDuration))),
      mSleepFunction(funcPtr) {
         if (mSleepFunction == nullptr) {
            mSleepFunction = std::bind(&AlarmClock::SleepUs, this, std::placeholders::_1);
         }
         mTimerThread = boost::thread(boost::bind(&AlarmClock::AlarmClockInterruptableThread, this));
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

      if (!mExpired.load()) {
         StopBackgroundThread();
      }
      mExpired.store(0);
      mCondition.notify_all();
   }

   int SleepTimeUs() {
      return kSleepTimeUsCount;
   }
   
   int SleepTimeMs() {
      return kSleepTimeMsCount;
   }

protected:

   void AlarmClockInterruptableThread() {
      boost::unique_lock<boost::mutex> lck(mMutex);
      do {
         mCondition.notify_all();
         int retVal = mSleepFunction(kSleepTimeUsCount);
         // If this was a normal exit then set expired to true, otherwise check
         // to see if it is an interrupt to exit the thread. 
         if (retVal == 0) {
            mExpired++;
         } else if (mExit){
            break;
         }
         
         try {
            mCondition.wait(lck); 
         } catch (boost::thread_interrupted e) {
            break;
         }
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
         boost::this_thread::sleep_for(boost::chrono::microseconds(t));
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
   std::atomic<bool> mExit;
   const int kSleepTime;
   const unsigned int kSleepTimeMsCount;
   const unsigned int kSleepTimeUsCount;
   std::function<unsigned int (unsigned int)> mSleepFunction;
   boost::thread mTimerThread;
   boost::mutex mMutex;
   boost::condition_variable mCondition;
};
