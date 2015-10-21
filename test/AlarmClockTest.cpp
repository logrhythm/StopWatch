/* 
 * File:   AlarmClockTest.cpp
 * Author: Craig Cogdill
 * Created: October 15, 2015 10:45am
 */

#include "AlarmClockTest.h"
#include "AlarmClock.h"
#include "StopWatch.h"
#include <chrono>

int kTimingLeewayMicro = 0;

namespace {
   template<typename T>
   void WaitForAlarmClockToExpire(AlarmClock<T>& alerter) {
      while (!alerter.has_expired());
   }

   template<typename Duration>
   unsigned int ConvertToMicroSeconds(Duration t) {
      return std::chrono::duration_cast<microseconds>(t).count();
   }
}

TEST_F(AlarmClockTest, DISABLED_usSimple) {
   unsigned int us = 500;
   StopWatch testTimer;
   AlarmClock<microseconds> alerter(us);
   WaitForAlarmClockToExpire(alerter);
   auto totalTime = testTimer.ElapsedUs();
   EXPECT_TRUE(us <= totalTime) << "AlarmClock didn't sleep for long enough. Slept for: " << totalTime << " us, should be longer than " << us;
   auto maxTime = us + kTimingLeewayMicro;
   EXPECT_TRUE(totalTime <= maxTime) << "AlarmClock took too long to expire. Took " << totalTime << " us. Should be less than " << maxTime;
}

TEST_F(AlarmClockTest, DISABLED_millisecondsUnderMinimumThresholdForSplitting) {
   unsigned int ms = 100;
   StopWatch testTimer;
   AlarmClock<milliseconds> alerter(ms);
   WaitForAlarmClockToExpire(alerter);
   auto totalTime = testTimer.ElapsedUs();
   std::cout << "total time =  " << totalTime << std::endl;
   auto msToMicro = ConvertToMicroSeconds(milliseconds(ms));
   EXPECT_TRUE(msToMicro <= totalTime) << "AlarmClock didn't sleep for long enough. Slept for: " << totalTime << " us, should be longer than " << msToMicro;
   auto maxTime = msToMicro + kTimingLeewayMicro;
   std::cout << "maxtime = " << maxTime << std::endl;
   EXPECT_TRUE(totalTime <= maxTime) << "AlarmClock took too long to expire. Took " << totalTime << " us. Should be less than " << maxTime;
}

TEST_F(AlarmClockTest, DISABLED_millisecondsAboveThresholdForSplitting) {
   unsigned int ms = 500;
   StopWatch testTimer;
   AlarmClock<milliseconds> alerter(ms);
   WaitForAlarmClockToExpire(alerter);
   auto totalTime = testTimer.ElapsedUs();
   std::cout << "total time =  " << totalTime << std::endl;
   auto msToMicro = ConvertToMicroSeconds(milliseconds(ms));
   EXPECT_TRUE(msToMicro <= totalTime) << "AlarmClock didn't sleep for long enough. Slept for: " << totalTime << " us, should be longer than " << msToMicro;
   auto maxTime = msToMicro + kTimingLeewayMicro;
   std::cout << "maxtime = " << maxTime << std::endl;
   EXPECT_TRUE(totalTime <= maxTime) << "AlarmClock took too long to expire. Took " << totalTime << " us. Should be less than " << maxTime;
}

TEST_F(AlarmClockTest, secondsSimple) {
   int sec = 1;
   StopWatch testTimer;
   AlarmClock<seconds> alerter(sec);
   WaitForAlarmClockToExpire(alerter);
   auto totalTime = testTimer.ElapsedUs();
   auto secToMicro = ConvertToMicroSeconds(seconds(sec));
   EXPECT_TRUE(secToMicro <= totalTime) << "AlarmClock didn't sleep for long enough. Slept for: " << totalTime << " sec, should be longer than " << secToMicro;
   auto maxTime = secToMicro + kTimingLeewayMicro;
   std::cout << "maxtime = " << maxTime << std::endl;
   EXPECT_TRUE(totalTime <= maxTime) << "AlarmClock took too long to expire. Took " << totalTime << " sec. Should be less than " << maxTime;
}

TEST_F(AlarmClockTest, DISABLED_secondsSimple2) {
   int sec = 2;
   StopWatch testTimer;
   AlarmClock<seconds> alerter(sec);
   WaitForAlarmClockToExpire(alerter);
   auto totalTime = testTimer.ElapsedUs();
   auto secToMicro = ConvertToMicroSeconds(seconds(sec));
   EXPECT_TRUE(secToMicro <= totalTime) << "AlarmClock didn't sleep for long enough. Slept for: " << totalTime << " sec, should be longer than " << secToMicro;
   auto maxTime = secToMicro + kTimingLeewayMicro;
   std::cout << "maxtime = " << maxTime << std::endl;
   EXPECT_TRUE(totalTime <= maxTime) << "AlarmClock took too long to expire. Took " << totalTime << " sec. Should be less than " << maxTime;
}
