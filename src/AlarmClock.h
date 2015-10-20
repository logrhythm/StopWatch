/* 
 * File:   LynxTimer.h
 * Author: Craig Cogdill
 * Created: October 15, 2015 10:45am
 */

#pragma once
#include <chrono>
#include <iostream>
#include <thread>
#include <czmq.h>
#include "StopWatch.h"

template<typename Duration> class LynxTimer {
public:
   explicit LynxTimer(int sleepDuration) : mExpired(false), kSleepTime(sleepDuration) {
      Sleep(kSleepTime);
      mExpired = true;
   }
   
   bool has_expired() {
      return mExpired;
   }

   void SmartSleep() {
      if (Duration(kSleepTime) <= std::chrono::microseconds(kSmallestIntervalInMS)) {
         Sleep(kSleepTime);
      } else {
         LynxTimer::SleepInIntervals();
      }
   }
   // LynxTimer& operator=(const LynxTimer& rhs);

protected:
   void Sleep(int sleepTime) {
      std::this_thread::sleep_for(Duration(kSleepTime)); 
   }

   void Sleep500MS() {
      std::this_thread::sleep_for(std::chrono::milliseconds(kSmallestIntervalInMS)); 
   }
   
   void SleepInIntervals() {
      if (Duration(kSleepTime) >= std::chrono::seconds(1)) {
         std::cout << ":" <<std::endl;
         size_t numberOfSleeps = kSleepTime / 2;
         while (KeepRunning() && numberOfSleeps > 0) {
            std::cout << "sleeping for 500..." <<std::endl;
            Sleep500MS();
            --numberOfSleeps; 
         }
      } else {
         std::cout << "sleeping ..." <<std::endl;
         Sleep(kSleepTime);
      }
      std::cout << " Sleeping for intervals " << std::endl;
   }

   bool KeepRunning() {
      return mExpired == false && !zctx_interrupted;
   }
   
private:

   bool mExpired;
   const int kSleepTime;
   const int kSmallestIntervalInMS = 500;
};


