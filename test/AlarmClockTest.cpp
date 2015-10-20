/* 
 * File:   AlarmClockTest.cpp
 * Author: Craig Cogdill
 * Created: October 15, 2015 10:45am
 */

#include "AlarmClockTest.h"
#include "AlarmClock.h"
#include "StopWatch.h"
#include <chrono>

#define kTimingLeeway std::chrono::milliseconds(10);

namespace {
   template<typename T>
   void WaitForAlarmClockToExpire(AlarmClock<T>& alerter) {
      while (!alerter.has_expired());
   }
}

TEST_F(AlarmClockTest, microsecondsSimple) {
   unsigned int us = 100;
   StopWatch testTimer;
   AlarmClock<std::chrono::microseconds> alerter(us);
   WaitForAlarmClockToExpire(alerter);
   auto totalTime = testTimer.ElapsedUs();
   EXPECT_TRUE(us <= totalTime) << "Slept for: " << us << ", Total Time: " << totalTime;
   EXPECT_TRUE(totalTime <= us + 10) << "The total time was too long";
}

TEST_F(AlarmClockTest, millisecondsSimple) {
   int ms = 100;
   auto preStart = std::chrono::steady_clock::now();
   AlarmClock<std::chrono::milliseconds> alerter(ms);
   WaitForAlarmClockToExpire(alerter);
   auto totalTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - preStart).count();
   EXPECT_TRUE(ms <= totalTime) << "Slept for:" << ms << ", Total Time: " << totalTime;
}


TEST_F(AlarmClockTest, secondsSimple) {
   int sec = 1;
   auto preStart = std::chrono::steady_clock::now();
   AlarmClock<std::chrono::seconds> alerter(sec);
   WaitForAlarmClockToExpire(alerter);
   auto totalTime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - preStart).count();
   EXPECT_TRUE(sec <= totalTime) << "Slept for: " << sec << ", Total Time: " << totalTime;
}
