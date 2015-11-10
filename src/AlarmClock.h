/* 
 * File:   AlarmClock.h
 * Author: Craig Cogdill
 * Created: October 15, 2015 10:45am
 */

#pragma once
#include <chrono>
#include <thread>
#include <atomic>
#include <iostream>
#include <future>
#include <functional>
#include "StopWatch.h"


template<typename Duration> class AlarmClock {
public:
   typedef std::chrono::microseconds microseconds;
   typedef std::chrono::milliseconds milliseconds;
   typedef std::chrono::seconds seconds;
   
   AlarmClock(unsigned int sleepDuration, std::function<void(unsigned int)> funcPtr = nullptr) : mExpired(false),
      kSleepTime(sleepDuration),
      kSleepTimeMsCount(ConvertToMillisecondsCount(Duration(sleepDuration))),
      kSleepTimeUsCount(ConvertToMicrosecondsCount(Duration(sleepDuration))),
      mSleepFunction(funcPtr) {
         if (mSleepFunction == nullptr) {
            std::cout << "Sleep function is nullptr" << std::endl;
            mSleepFunction = std::bind(&AlarmClock::SleepUs, this, std::placeholders::_1);
         }
         mExited = std::async(std::launch::async, &AlarmClock::AlarmClockThread,this);
   }
   
   virtual ~AlarmClock() {
      StopBackgroundThread();
   }
   
   bool Expired() {
      return mExpired.load();
   }

   void Reset() {
      if (!mExpired.load()) {
         StopBackgroundThread();
      }
      mExpired.store(false); 
      mExited = std::async(std::launch::async, &AlarmClock::AlarmClockThread, this);
   }

   int SleepTimeUs() {
      return kSleepTimeUsCount;
   }
   
   int SleepTimeMs() {
      return kSleepTimeMsCount;
   }

protected:

   void AlarmClockThread() {
      SleepTimeIsBelow500ms() ? AlarmClock::SleepForFullAmount() : AlarmClock::SleepInIntervals();
   }
  
   void StopBackgroundThread() {
      mExpired.store(true);
      mExited.wait();
   }

   bool SleepTimeIsBelow500ms() {
      return Duration(kSleepTime) <= microseconds(kSmallestIntervalInUS);
   }

   void SleepForFullAmount() {
      std::cout << "Sleeping for full amount" << std::endl;
      mSleepFunction(kSleepTimeUsCount);
      std::cout << "Done sleeping" << std::endl;
      mExpired.store(true);
   }

   void SleepUs(unsigned int t) {
      std::cout << "We sleepin..." << std::endl;
      std::this_thread::sleep_for(microseconds(t));
   }
   
   // If 500ms is NOT an even divisor of the amount of sleep
   //    time given, should sleep for every multiple of 500ms
   //    that divides, then check the remaining time and just
   //    sleep for only that last bit.
   //
   // If 500ms IS an even divisor of the sleep time, sleep for 
   //    all but one multiple of 500ms. There is overhead induced
   //    by the while loop, so if the full amount of time is slept,
   //    the precision of the alarm clock will suffer.
   size_t GetNumberOfSleepIntervals() {
      return (kSleepTimeMsCount % kSmallestIntervalInMS == 0) ?
             ((kSleepTimeMsCount/kSmallestIntervalInMS) - 1) :
             (kSleepTimeMsCount/kSmallestIntervalInMS);
   }
  
   void SleepForRemainder(const unsigned int& currentSleptFor) {
      if (currentSleptFor < kSleepTimeUsCount) {
         mSleepFunction(kSleepTimeUsCount - currentSleptFor);
      }
   } 

   void SleepInIntervals() {
      StopWatch timer;
      size_t numberOfSleeps = GetNumberOfSleepIntervals();
      while (KeepRunning() && numberOfSleeps > 0) {
         mSleepFunction(kSmallestIntervalInUS);
         --numberOfSleeps; 
      }
      auto currentSleptFor = timer.ElapsedUs();
      if (KeepRunning()) {
         SleepForRemainder(currentSleptFor);
      }
      mExpired.store(true);
   }

   bool KeepRunning() {
      return !mExpired.load();
   }
   
   unsigned int ConvertToMillisecondsCount(Duration t) {
      return std::chrono::duration_cast<milliseconds>(t).count();
   }
   
   unsigned int ConvertToMicrosecondsCount(Duration t) {
      return std::chrono::duration_cast<microseconds>(t).count();
   }
   
   microseconds ConvertToMicroseconds(Duration t) {
      return std::chrono::duration_cast<microseconds>(t);
   }
   
private:

   std::atomic<bool> mExpired;
   const int kSleepTime;
   const unsigned int kSleepTimeMsCount;
   const unsigned int kSleepTimeUsCount;
   std::function<void(unsigned int)> mSleepFunction;
   std::future<void> mExited;
   const unsigned int kSmallestIntervalInMS = 500;
   const unsigned int kSmallestIntervalInUS = 500 * 1000; // 500ms
};


