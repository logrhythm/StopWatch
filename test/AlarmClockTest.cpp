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
// experimentally. It takes around 70-80 
// microseconds on average to get through
// one while loop of 500ms sleep. 
int kTimingLeeway = 200;

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

      if (timeInMicro <= k500MsInMicro) {
          return kTimingLeeway;
      } else if (timeInMicro % k500MsInMicro != 0) {
         return kTimingLeeway * (timeInMicro / k500MsInMicro + 1);
      } else {
         return kTimingLeeway * (timeInMicro / k500MsInMicro);
      }
   }
}

TEST_F(AlarmClockTest, DISABLED_GetTimingLeewayTest) {
   GetTimingLeeway(milliseconds(5));
   GetTimingLeeway(seconds(5));
   GetTimingLeeway(microseconds(5));
   GetTimingLeeway(milliseconds(15));
}

TEST_F(AlarmClockTest, DISABLED_usSimple) {
   int us = 900;
   StopWatch testTimer;
   AlarmClock<microseconds> alerter(us);
   WaitForAlarmClockToExpire(alerter);
   int totalTime = testTimer.ElapsedUs();
   std::cout << "Timeout was set for " << us << " microseconds. Actually slept for " << totalTime << " microseconds." << std::endl;
   std::cout << "Time leeway = " << GetTimingLeeway(microseconds(us)) << std::endl;
   EXPECT_TRUE(us <= totalTime) << "AlarmClock didn't sleep for long enough. Slept for: " << totalTime << " us, should be longer than " << us;
   auto maxTime = us + GetTimingLeeway(microseconds(us));
   EXPECT_TRUE(totalTime <= maxTime) << "AlarmClock took too long to expire. Took " << totalTime << " us. Should be less than " << maxTime;
}

TEST_F(AlarmClockTest, millisecondsUnderMinimumThresholdForSplitting) {
   unsigned int ms = 1000;
   StopWatch testTimer;
   AlarmClock<milliseconds> alerter(ms);
   WaitForAlarmClockToExpire(alerter);
   auto totalTime = testTimer.ElapsedUs();
   std::cout << "Timeout was set for " << ConvertToMicroSeconds(milliseconds(ms)) << " microseconds. Actually slept for " << totalTime << " microseconds." << std::endl;
   std::cout << "total time =  " << totalTime << std::endl;
   auto msToMicro = ConvertToMicroSeconds(milliseconds(ms));
   std::cout << "Time leeway = " << GetTimingLeeway(milliseconds(ms)) << std::endl;
   EXPECT_TRUE(msToMicro <= totalTime) << "AlarmClock didn't sleep for long enough. Slept for: " << totalTime << " us, should be longer than " << msToMicro;
   auto maxTime = msToMicro + GetTimingLeeway(milliseconds(ms));
   std::cout << "maxtime = " << maxTime << std::endl;
   EXPECT_TRUE(totalTime <= maxTime) << "AlarmClock took too long to expire. Took " << totalTime << " us. Should be less than " << maxTime;
}

TEST_F(AlarmClockTest, StopWatchVSAlarmClock) {
   StopWatch stopWatch;
   while (stopWatch.ElapsedSec() < 4) {
      std::this_thread::sleep_for(milliseconds(1));
   }
   auto stopWatchFinal = stopWatch.ElapsedUs();

   AlarmClock<seconds> alerter(4);
   WaitForAlarmClockToExpire(alerter);
   auto alarmClockFinal = stopWatch.ElapsedUs() - stopWatchFinal;

   std::cout << "AlarmClock: " << alarmClockFinal << "      StopWatch:  " << stopWatchFinal << std::endl;

}

// TEST_F(AlarmClockTest, secondsSimple) {
//    int sec = 1;
//    StopWatch testTimer;
//    AlarmClock<seconds> alerter(sec);
//    WaitForAlarmClockToExpire(alerter);
//    auto totalTime = testTimer.ElapsedUs();
//    std::cout << "Timeout was set for " << ConvertToMicroSeconds(seconds(sec)) << " seconds. Actually slept for " << totalTime << " seconds." << std::endl;
//    auto secToMicro = ConvertToMicroSeconds(seconds(sec));
//    EXPECT_TRUE(secToMicro <= totalTime) << "AlarmClock didn't sleep for long enough. Slept for: " << totalTime << " sec, should be longer than " << secToMicro;
//    auto maxTime = secToMicro + (kTimingLeeway * sec);
//    std::cout << "maxtime = " << maxTime << std::endl;
//    EXPECT_TRUE(totalTime <= maxTime) << "AlarmClock took too long to expire. Took " << totalTime << " sec. Should be less than " << maxTime;
// }

// TEST_F(AlarmClockTest, DISABLED_secondsSimple10) {
//    int sec = 10;
//    StopWatch testTimer;
//    AlarmClock<seconds> alerter(sec);
//    WaitForAlarmClockToExpire(alerter);
//    auto totalTime = testTimer.ElapsedUs();
//    std::cout << "Timeout was set for " << ConvertToMicroSeconds(seconds(sec)) << " seconds. Actually slept for " << totalTime << " seconds." << std::endl;
//    auto secToMicro = ConvertToMicroSeconds(seconds(sec));
//    EXPECT_TRUE(secToMicro <= totalTime) << "AlarmClock didn't sleep for long enough. Slept for: " << totalTime << " sec, should be longer than " << secToMicro;
//    auto maxTime = secToMicro + (kTimingLeeway * sec);
//    std::cout << "maxtime = " << maxTime << std::endl;
//    EXPECT_TRUE(totalTime <= maxTime) << "AlarmClock took too long to expire. Took " << totalTime << " sec. Should be less than " << maxTime;
// }

// TEST_F(AlarmClockTest, DISABLED_millisecondsNotAMultipleOf500) {
//    int ms = 1300;
//    StopWatch testTimer;
//    AlarmClock<milliseconds> alerter(ms);
//    WaitForAlarmClockToExpire(alerter);
//    auto totalTime = testTimer.ElapsedUs();
//    auto msToMicro = ConvertToMicroSeconds(milliseconds(ms));
//    std::cout << "Timeout was set for " << msToMicro << " microseconds. Actually slept for " << totalTime << " seconds." << std::endl;
//    EXPECT_TRUE(msToMicro <= totalTime) << "AlarmClock didn't sleep for long enough. Slept for: " << totalTime << " sec, should be longer than " << msToMicro;
//    auto maxTime = msToMicro + kTimingLeeway;
//    std::cout << "maxtime = " << maxTime << std::endl;
//    EXPECT_TRUE(totalTime <= maxTime) << "AlarmClock took too long to expire. Took " << totalTime << " sec. Should be less than " << maxTime;
// }
