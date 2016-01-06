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
      std::cout << "DESTRUCTOR " << boost::this_thread::get_id() << ": Setting mExit to TRUE" << std::endl;
      mExit.store(true);
      std::cout << "DESTRUCTOR " << boost::this_thread::get_id() << ": Calling stop background thread" << std::endl;
      StopBackgroundThread();
      std::cout << "DESTRUCTOR " << boost::this_thread::get_id() << ": Finished!" << std::endl;
   }
   
   bool Expired() {
      return mExpired.load();
   }

   void Reset() {
      boost::unique_lock<boost::mutex> lck(mMutex);

      if (!mExpired.load()) {
         std::cout << "RESET " << boost::this_thread::get_id() << ": Stopping background thread" << std::endl;
         StopBackgroundThread();
      }
      std::cout << "RESET " << boost::this_thread::get_id() << ": Setting mExpired to 0" << std::endl;
      mExpired.store(0);
      std::cout << "RESET " << boost::this_thread::get_id() << ": Notifying all threads" << std::endl;
      mCondition.notify_all();
      std::cout << "RESET " << boost::this_thread::get_id() << ": Finished!" << std::endl;
   }

   int SleepTimeUs() {
      return kSleepTimeUsCount;
   }
   
   int SleepTimeMs() {
      return kSleepTimeMsCount;
   }

protected:

   void AlarmClockInterruptableThread() {
      std::cout << "THREAD " << boost::this_thread::get_id() << ": Creating lock" << std::endl;
      boost::unique_lock<boost::mutex> lck(mMutex);

      do {
         std::cout << "THREAD " << boost::this_thread::get_id() << ": Calling sleep function" << std::endl;
         unsigned int retVal = mSleepFunction(kSleepTimeUsCount);
         std::cout << "THREAD " << boost::this_thread::get_id() << ": Sleep function finished, incrementing expired and exiting" << std::endl;

         if (retVal == 0) {
            std::cout << "THREAD " << boost::this_thread::get_id() << ": Time expired!" << std::endl;
            // Expired, should increment mExpired
            mExpired++;
         } else if (mExit) {
            std::cout << "THREAD " << boost::this_thread::get_id() << ": Interrupted! Exit is true!" << std::endl;
            break;
         }

         // Wait condition must be in separate interrupt try catch to ensure
         // that if the sleep is interrupted, the thread still waits for the 
         // signal to either exit or continue.
         try {
            std::cout << "THREAD " << boost::this_thread::get_id() << ": Waiting on lock" << std::endl;
            mCondition.wait(lck); 
         } catch (boost::thread_interrupted e) {
            std::cout << "THREAD " << boost::this_thread::get_id() << ": Interrupted while waiting on lock, breaking." << std::endl;
            break;
         }
      } while (!mExit);
      std::cout << "THREAD " << boost::this_thread::get_id() << ": Out of loop! Exiting" << std::endl;
   }
  
   void StopBackgroundThread() {
      std::cout << "STOPPER " << boost::this_thread::get_id() << ": Interrupting thread" << std::endl;
      mTimerThread.interrupt();
      // Check to see if the thread is joinable and only join if it is supposed
      // to exit.
      std::cout << "STOPPER " << boost::this_thread::get_id() << ": Checking to see if the threads should be joined" << std::endl;
      if (mTimerThread.joinable() && mExit) {
         std::cout << "STOPPER " << boost::this_thread::get_id() << ": Joining threads" << std::endl;
         mTimerThread.join();
      }   
      std::cout << "STOPPER" << boost::this_thread::get_id() << ": Exiting" << std::endl;
   }

   unsigned int SleepUs(unsigned int t) {
      try {
         std::cout << "SLEEPER " << boost::this_thread::get_id() << ": Initiating sleep" << std::endl;
         boost::this_thread::sleep_for(boost::chrono::microseconds(t));
         std::cout << "SLEEPER " << boost::this_thread::get_id() << ": Returning 0" << std::endl;
         return 0;
      } catch (boost::thread_interrupted e) {
         std::cout << "SLEEPER " << boost::this_thread::get_id() << ": Interrupted! Returning 1" << std::endl;
         return 1;
      }
      std::cout << "SLEEPER " << boost::this_thread::get_id() << ": Exiting!" << std::endl;
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
