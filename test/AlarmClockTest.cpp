/* 
 * File:   AlarmClockTest.cpp
 * Author: Craig Cogdill
 * Created: October 15, 2015 10:45am
 */

#include "AlarmClockTest.h"
#include "AlarmClock.h"
#include "StopWatch.h"
#include <chrono>

// The timing leeway was determined
// experimentally. It takes around 130-160 
// microseconds on average to get through
// one while loop of 500ms sleep. 
int kTimingLeeway = 250;

namespace {
   template<typename T>
   void WaitForAlarmClockToExpire(AlarmClock<T>& alerter) {
      while (!alerter.has_expired());
   }

   template<typename Duration>
   unsigned int ConvertToMicroSeconds(Duration t) {
      return std::chrono::duration_cast<microseconds>(t).count();
   }

   template <typename Duration>
   int GetTimingLeeway(Duration t) {

      auto timeInMicro = ConvertToMicroSeconds(t);
      auto k500MsInMicro = ConvertToMicroSeconds(milliseconds(500));

      // One sleep of 500ms or less => 200us leeway
      if (timeInMicro <= k500MsInMicro) {
          return kTimingLeeway;
      // One leeway of 200us for every loop of 500ms, plus
      //    one extra for the leftover time    
      } else if (timeInMicro % k500MsInMicro != 0) {
         return kTimingLeeway * (timeInMicro / k500MsInMicro + 1);
      // One leeway of 200us for every loop of 500ms
      } else {
         return kTimingLeeway * (timeInMicro / k500MsInMicro);
      }
   }
}

TEST_F(AlarmClockTest, microsecondsLessThan500ms) {
   int us = 900;
   StopWatch testTimer;
   AlarmClock<microseconds> alerter(us);
   WaitForAlarmClockToExpire(alerter);
   int totalTime = testTimer.ElapsedUs();
   auto maxTime = us + GetTimingLeeway(microseconds(us));
   EXPECT_TRUE(us <= totalTime) << "AlarmClock didn't sleep for long enough. Slept for: " << totalTime << " us, should be longer than " << us;
   EXPECT_TRUE(totalTime <= maxTime) << "AlarmClock took too long to expire. Took " << totalTime << " us. Should be less than " << maxTime;
   std::cout << "Timeout was set for " << us << " us. Actually slept for " << totalTime << " us. Max timeout: " << maxTime << std::endl;
}

TEST_F(AlarmClockTest, microsecondsGreaterThan500ms) {
   int us = 600000;
   StopWatch testTimer;
   AlarmClock<microseconds> alerter(us);
   WaitForAlarmClockToExpire(alerter);
   int totalTime = testTimer.ElapsedUs();
   auto maxTime = us + GetTimingLeeway(microseconds(us));
   EXPECT_TRUE(us <= totalTime) << "AlarmClock didn't sleep for long enough. Slept for: " << totalTime << " us, should be longer than " << us;
   EXPECT_TRUE(totalTime <= maxTime) << "AlarmClock took too long to expire. Took " << totalTime << " us. Should be less than " << maxTime;
   std::cout << "Timeout was set for " << us << " us. Actually slept for " << totalTime << " us. Max timeout: " << maxTime << std::endl;
}

TEST_F(AlarmClockTest, millisecondsLessThan500) {
   unsigned int ms = 100;
   StopWatch testTimer;
   AlarmClock<milliseconds> alerter(ms);
   WaitForAlarmClockToExpire(alerter);
   auto totalTime = testTimer.ElapsedUs();
   auto msToMicro = ConvertToMicroSeconds(milliseconds(ms));
   EXPECT_TRUE(msToMicro <= totalTime) << "AlarmClock didn't sleep for long enough. Slept for: " << totalTime << " us, should be longer than " << msToMicro;
   auto maxTime = msToMicro + GetTimingLeeway(milliseconds(ms));
   EXPECT_TRUE(totalTime <= maxTime) << "AlarmClock took too long to expire. Took " << totalTime << " us. Should be less than " << maxTime;
   std::cout << "Timeout was set for " << msToMicro << " us. Actually slept for " << totalTime << " us. Max timeout: " << maxTime << std::endl;
}

TEST_F(AlarmClockTest, oneSecondInMilliseconds) {
   unsigned int ms = 1000;
   StopWatch testTimer;
   AlarmClock<milliseconds> alerter(ms);
   WaitForAlarmClockToExpire(alerter);
   auto totalTime = testTimer.ElapsedUs();
   auto msToMicro = ConvertToMicroSeconds(milliseconds(ms));
   EXPECT_TRUE(msToMicro <= totalTime) << "AlarmClock didn't sleep for long enough. Slept for: " << totalTime << " us, should be longer than " << msToMicro;
   auto maxTime = msToMicro + GetTimingLeeway(milliseconds(ms));
   EXPECT_TRUE(totalTime <= maxTime) << "AlarmClock took too long to expire. Took " << totalTime << " us. Should be less than " << maxTime;
   std::cout << "Timeout was set for " << msToMicro << " us. Actually slept for " << totalTime << " us. Max timeout: " << maxTime << std::endl;
}

TEST_F(AlarmClockTest, millisecondsNotDivisibleBy500) {
   unsigned int ms = 1300;
   StopWatch testTimer;
   AlarmClock<milliseconds> alerter(ms);
   WaitForAlarmClockToExpire(alerter);
   auto totalTime = testTimer.ElapsedUs();
   auto msToMicro = ConvertToMicroSeconds(milliseconds(ms));
   EXPECT_TRUE(msToMicro <= totalTime) << "AlarmClock didn't sleep for long enough. Slept for: " << totalTime << " us, should be longer than " << msToMicro;
   auto maxTime = msToMicro + GetTimingLeeway(milliseconds(ms));
   EXPECT_TRUE(totalTime <= maxTime) << "AlarmClock took too long to expire. Took " << totalTime << " us. Should be less than " << maxTime;
   std::cout << "Timeout was set for " << msToMicro << " us. Actually slept for " << totalTime << " us. Max timeout: " << maxTime << std::endl;
}

TEST_F(AlarmClockTest, secondsSimple) {
   int sec = 1;
   StopWatch testTimer;
   AlarmClock<seconds> alerter(sec);
   WaitForAlarmClockToExpire(alerter);
   auto totalTime = testTimer.ElapsedUs();
   auto secToMicro = ConvertToMicroSeconds(seconds(sec));
   EXPECT_TRUE(secToMicro <= totalTime) << "AlarmClock didn't sleep for long enough. Slept for: " << totalTime << " sec, should be longer than " << secToMicro;
   auto maxTime = secToMicro + (kTimingLeeway * sec);
   EXPECT_TRUE(totalTime <= maxTime) << "AlarmClock took too long to expire. Took " << totalTime << " sec. Should be less than " << maxTime;
   std::cout << "Timeout was set for " << secToMicro << " us. Actually slept for " << totalTime << " us. Max timeout: " << maxTime << std::endl;
}

TEST_F(AlarmClockTest, 2VariableConstructor_ShouldRun) {
   unsigned int ms = 500;
   StopWatch timer;
   AlarmClock<milliseconds> alerter(ms, false);
   WaitForAlarmClockToExpire(alerter);
   auto totalTime = timer.ElapsedUs();
   auto secToMicro = ConvertToMicroSeconds(milliseconds(ms));
   EXPECT_TRUE(alerter.has_expired());
   EXPECT_TRUE(secToMicro <= totalTime) << "AlarmClock didn't sleep for long enough. Slept for: " << totalTime << " sec, should be longer than " << secToMicro;
   auto maxTime = secToMicro + GetTimingLeeway(milliseconds(ms));
   EXPECT_TRUE(totalTime <= maxTime) << "AlarmClock took too long to expire. Took " << totalTime << " sec. Should be less than " << maxTime;
   std::cout << "Timeout was set for " << secToMicro << " us. Actually slept for " << totalTime << " us. Max timeout: " << maxTime << std::endl;
}

TEST_F(AlarmClockTest, 2VariableConstructor_ShouldExitImmediately) {
   unsigned int ms = 5000;
   StopWatch timer;
   AlarmClock<milliseconds> alerter(ms, true);
   WaitForAlarmClockToExpire(alerter);
   auto totalTime = timer.ElapsedUs();
   EXPECT_TRUE(alerter.has_expired());
   auto minimumTimingLeeway = GetTimingLeeway(microseconds(1));
   EXPECT_TRUE(totalTime <= static_cast<unsigned int>(minimumTimingLeeway)) << "AlarmClock slept way too long. Slept for: " << totalTime << " us, should be less than " << minimumTimingLeeway;
   std::cout << "Timeout was set for " << ms << " ms. Actually slept for " << totalTime << " us." << std::endl;
}

TEST_F(AlarmClockTest, AlarmClockVSStopWatch) {
   size_t swCount = 0;
   size_t acCount = 0;
   StopWatch timer;
   while(timer.ElapsedSec() < 1) {
      ++swCount;
   }
   auto swTime = timer.ElapsedUs();
   AlarmClock<seconds> alerter(1);
   while(!alerter.has_expired()) {
      ++acCount;
   }
   auto acTime = timer.ElapsedUs();

   std::cout << "StopWatch count: "<<swCount<<"     AlarmClock count: "<<acCount<<std::endl;
   std::cout << "StopWatch took "<<swTime<<" us...     AlarmClock took "<<acTime-swTime<<std::endl;
}