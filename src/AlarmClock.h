/* 
 * File:   AlarmClock.h
 * Author: Amanda Carbonari
 * Created: January 5, 2016 4:35pm
 */

#pragma once
#include <chrono>
#include <boost/thread.hpp>
//#include <thread>
#include <atomic>
//#include <future>
#include <functional>
//#include "StopWatch.h"


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
         std::cout << "Creating sleep function" << std::endl;
         if (mSleepFunction == nullptr) {
            std::cout << "Null sleep function, using default" << std::endl;
            mSleepFunction = std::bind(&AlarmClock::SleepUs, this, std::placeholders::_1);
         }
         std::cout << "Creating thread" << std::endl;
         mTimerThread = boost::thread(boost::bind(&AlarmClock::AlarmClockInterruptableThread, this));
         std::cout << "Finished constructing" << std::endl;
   }

   virtual ~AlarmClock() {
      mExit.store(true);
      mCondition.notify_all();
      StopBackgroundThread();
   }
   
   bool Expired() {
      return mExpired.load();
   }

   void Reset() {
      std::cout << "RESET " << boost::this_thread::get_id() << ": creating lock" << std::endl;
      boost::unique_lock<boost::mutex> lck(mMutex);

      if (!mExpired.load()) {
         std::cout << "RESET " << boost::this_thread::get_id() << ": stopping background thread" << std::endl;
         StopBackgroundThread();
      }
      //mTimerThread -> mSleepFunction();
      std::cout << "RESET " << boost::this_thread::get_id() << ": Setting expired to 0" << std::endl;
      mExpired.store(0);
      std::cout << "RESET " << boost::this_thread::get_id() << ": Notifying all" << std::endl;
      mCondition.notify_all();

      //while (mExpired == 0 || !mExit) { 
      //   std::cout << "RESET " << boost::this_thread::get_id() << ": Waiting for lock" << std::endl;
      //   mCondition.wait(lck); 
      //}
      std::cout << "RESET " << boost::this_thread::get_id() << ": all done" << std::endl;
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
      std::cout << "THREAD " << boost::this_thread::get_id() << ": Starting while loop" << std::endl;
      do {
         std::cout << "THREAD " << boost::this_thread::get_id() << ": releasing lock" << std::endl;
         mCondition.notify_all();
         std::cout << "THREAD " << boost::this_thread::get_id() << ": Calling sleep function" << std::endl;
         int retVal = mSleepFunction(kSleepTimeUsCount);
         // If this was a normal exit then set expired to true. 
         std::cout << "THREAD " << boost::this_thread::get_id() << ": Checking return value" << std::endl;
         if (retVal == 0) {
            std::cout << "THREAD " << boost::this_thread::get_id() << ": Time expired" << std::endl;
            mExpired++;
         } else if (mExit){
            std::cout << "THREAD " << boost::this_thread::get_id() << ": Interrupted and should exit" << std::endl;
            break;
         } else if (retVal == -1) {
            std::cout << "THREAD " << boost::this_thread::get_id() << ": Fake Sleep detected" << std::endl;
            mExpired++;
         }
         std::cout << "THREAD " << boost::this_thread::get_id() << ": end of loop, waiting on lock" << std::endl;
         while(mExpired > 0) { 
            std::cout << "THREAD " << boost::this_thread::get_id() << ": Waiting for lock" << std::endl;
            try {
               mCondition.wait(lck); 
            } catch (boost::thread_interrupted e) {
               std::cout << "THREAD " << boost::this_thread::get_id() << ": wait got interrupted" << std::endl;
               break;
            }
         }
         std::cout << "THREAD " << boost::this_thread::get_id() << ": got lock! Starting loop again" << std::endl;
      } while (!mExit);
      std::cout << "THREAD " << boost::this_thread::get_id() << ": exiting while loop" << std::endl;
   }
  
   void StopBackgroundThread() {
      std::cout << "STOPPER " << boost::this_thread::get_id() << ": interrupting thread" << std::endl;
      mTimerThread.interrupt();
      std::cout << "STOPPER " << boost::this_thread::get_id() << ": checking if joinable" << std::endl;
      if (mTimerThread.joinable() && mExit) {
         std::cout << "STOPPER " << boost::this_thread::get_id() << ": joining thread" << std::endl;
         mTimerThread.join();
      }   
   }

   unsigned int SleepUs(unsigned int t) {
      try {
         std::cout << "SLEEP FUNCTION " << boost::this_thread::get_id() << ": calling boost sleep" << std::endl;
         boost::this_thread::sleep_for(boost::chrono::microseconds(t));
         std::cout << "SLEEP FUNCTION " << boost::this_thread::get_id() << ": sleep ended" << std::endl;
         return 0;
      } catch (boost::thread_interrupted e) {
         std::cout << "SLEEP FUNCTION " << boost::this_thread::get_id() << ": interrupted" << std::endl;
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
   boost::unique_lock<boost::mutex> mLock;
   boost::condition_variable mCondition;
};
