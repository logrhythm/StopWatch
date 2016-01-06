/* 
 * File:   AlarmClock.h
 * Author: Amanda Carbonari
 * Created: January 5, 2016 4:35pm
 * Description: Utilizes the boost library to create a singleton interruptable thread
 *   that "communicates" with the main thread via a mutex, locks and condition variables.
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
   
   AlarmClock(unsigned int sleepDuration, std::function<void (unsigned int)> funcPtr = nullptr) : mExpired(0),
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
      mSleepFunction(kSleepTimeUsCount);
   }
  
   void StopBackgroundThread() {
      mTimerThread.interrupt();
      // Check to see if the thread is joinable and only join if it is supposed
      // to exit.
      if (mTimerThread.joinable() && mExit) {
         mTimerThread.join();
      }   
   }

   void SleepUs(unsigned int t) {
      boost::unique_lock<boost::mutex> lck(mMutex);
      do {
         try {
            boost::this_thread::sleep_for(boost::chrono::microseconds(t));
            mExpired++;
         } catch (boost::thread_interrupted e) {
            if (mExit) {
               break;
            }
         }

         // Wait condition must be in separate interrupt try catch to ensure
         // that if the sleep is interrupted, the thread still waits for the 
         // signal to either exit or continue.
         try {
            mCondition.wait(lck); 
         } catch (boost::thread_interrupted e) {
            break;
         }
      } while (!mExit);
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
   std::function<void (unsigned int)> mSleepFunction;
   boost::thread mTimerThread;
   boost::mutex mMutex;
   boost::condition_variable mCondition;
};
