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

   bool print = false;
   
   AlarmClock(unsigned int sleepDuration, std::function<unsigned int (unsigned int)> funcPtr = nullptr) : mExpired(0),
      mExit(false),
      mReset(false),
      kSleepTime(sleepDuration),
      kSleepTimeMsCount(ConvertToMillisecondsCount(Duration(sleepDuration))),
      kSleepTimeUsCount(ConvertToMicrosecondsCount(Duration(sleepDuration))),
      mSleepFunction(funcPtr) {
         if (print) {
            std::cout << "CONSTRUCTOR " << boost::this_thread::get_id() << ": checking sleep function" << std::endl;
         }
         if (mSleepFunction == nullptr) {
            if (print) {
               std::cout << "CONSTRUCTOR " << boost::this_thread::get_id() << ": setting sleep function to default" << std::endl;
            }
            mSleepFunction = std::bind(&AlarmClock::SleepUs, this, std::placeholders::_1);
         }
         if (print) {
            std::cout << "CONSTRUCTOR " << boost::this_thread::get_id() << ": creating background thread" << std::endl;
         }
         mTimerThread = boost::thread(&AlarmClock::AlarmClockInterruptableThread, this);
         if (print) {
            std::cout << "CONSTRUCTOR " << boost::this_thread::get_id() << ": finished!" << std::endl;
         }
   }

   virtual ~AlarmClock() {
      if (print) {
         std::cout << "DESTRUCTOR " << boost::this_thread::get_id() << ": Setting mExit to TRUE" << std::endl;
      }
      mExit.store(true);
      if (print) {
         std::cout << "DESTRUCTOR " << boost::this_thread::get_id() << ": Calling stop background thread" << std::endl;
      }
      StopBackgroundThread();
      if (print) {
         std::cout << "DESTRUCTOR " << boost::this_thread::get_id() << ": Finished!" << std::endl;
      }
   }
   
   bool Expired() {
      return mExpired.load();
   }

   void Reset() {
      if (print) {
         std::cout << "RESET " << boost::this_thread::get_id() << ": Creating lock" << std::endl;
      }
      boost::unique_lock<boost::mutex> lck(mMutex);
      mReset.store(true);
      if (!mExpired.load()) {
         if (print) {
            std::cout << "RESET " << boost::this_thread::get_id() << ": Stopping background thread" << std::endl;
         }
         StopBackgroundThread();
      }
      if (print) {
         std::cout << "RESET " << boost::this_thread::get_id() << ": Setting mExpired to 0" << std::endl;
      }
      mExpired.store(0);
      if (print) {
         std::cout << "RESET " << boost::this_thread::get_id() << ": Notifying all threads" << std::endl;
      }
      {mCondition.notify_all();}
      if (print) {
         std::cout << "RESET " << boost::this_thread::get_id() << ": Finished!" << std::endl;
      }
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
         if (print) {
            std::cout << "THREAD " << boost::this_thread::get_id() << ": Calling sleep function" << std::endl;
         }
         unsigned int retVal = mSleepFunction(kSleepTimeUsCount);
         if (print) {
            std::cout << "THREAD " << boost::this_thread::get_id() << ": Sleep function finished" << std::endl;
         }

         if (retVal == 0) {
            if (print) {
               std::cout << "THREAD " << boost::this_thread::get_id() << ": Time expired! " << std::endl;
            }
            // Expired, should increment mExpired
            mExpired++;
            if (print) {
               std::cout << "THREAD " << boost::this_thread::get_id() << ": mExpired = " << mExpired << std::endl; 
            }
         } else if (mExit) {
            if (print) {
               std::cout << "THREAD " << boost::this_thread::get_id() << ": Interrupted! Exit is true!" << std::endl;
            }
            break;
         }

         if (!mReset) {
            if (print) {
               std::cout << "THREAD " << boost::this_thread::get_id() << ": Creating lock" << std::endl;
            }
            boost::unique_lock<boost::mutex> lck(mMutex);
            try {
               if (print) {
                  std::cout << "THREAD " << boost::this_thread::get_id() << ": Waiting on lock" << std::endl;
               }
               mCondition.wait(lck); 
            } catch (boost::thread_interrupted e) {
               if (print) {
                  std::cout << "THREAD " << boost::this_thread::get_id() << ": Interrupted while waiting on lock, reentering loop." << std::endl;
               }
            }
         }
         mReset.store(false);
      } while (!mExit);
      if (print) {
         std::cout << "THREAD " << boost::this_thread::get_id() << ": Out of loop! Exiting" << std::endl;
      }
   }
  
   void StopBackgroundThread() {
      if (print) {
         std::cout << "STOPPER " << boost::this_thread::get_id() << ": Interrupting thread" << std::endl;
      }
      mTimerThread.interrupt();
      // Check to see if the thread is joinable and only join if it is supposed
      // to exit.
      if (print) {
         std::cout << "STOPPER " << boost::this_thread::get_id() << ": Checking to see if the threads should be joined" << std::endl;
      }
      if (mTimerThread.joinable() && mExit) {
         if (print) {
            std::cout << "STOPPER " << boost::this_thread::get_id() << ": Joining threads" << std::endl;
         }
         mTimerThread.join();
      }   
      if (print) {
         std::cout << "STOPPER " << boost::this_thread::get_id() << ": Exiting" << std::endl;
      }
   }

   unsigned int SleepUs(unsigned int t) {
      try {
         if (print) {
            std::cout << "SLEEPER " << boost::this_thread::get_id() << ": Initiating sleep" << std::endl;
         }
         std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
         boost::this_thread::sleep_for(boost::chrono::microseconds(t));
         std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
         auto sleep_time = std::chrono::duration_cast<microseconds>(t2-t1).count();
         if (true) {
            std::cout << "SLEEPER " << boost::this_thread::get_id() << ": Time slept = " << sleep_time << std::endl;
            //std::cout << "SLEEPER " << boost::this_thread::get_id() << ": Returning 0" << std::endl;
         }
         return 0;
      } catch (boost::thread_interrupted e) {
         if (print) {
            std::cout << "SLEEPER " << boost::this_thread::get_id() << ": Interrupted! Returning 1" << std::endl;
         }
         return 1;
      }
      if (print) {
         std::cout << "SLEEPER " << boost::this_thread::get_id() << ": Exiting!" << std::endl;
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
   std::atomic<bool> mReset;
   const int kSleepTime;
   const unsigned int kSleepTimeMsCount;
   const unsigned int kSleepTimeUsCount;
   std::function<unsigned int (unsigned int)> mSleepFunction;
   boost::thread mTimerThread;
   boost::mutex mMutex;
   boost::condition_variable mCondition;
};
