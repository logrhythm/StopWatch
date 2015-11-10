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
   typedef std::chrono::microseconds microseconds;
   typedef std::chrono::milliseconds milliseconds;
   typedef std::chrono::seconds seconds;
   
   template<typename T>
   void WaitForAlarmClockToExpire(AlarmClock<T>& alerter) {
      while (!alerter.Expired());
   }

   template<typename Duration>
   unsigned int ConvertToMicroSeconds(Duration t) {
      return std::chrono::duration_cast<microseconds>(t).count();
   }
   
   template<typename Duration>
   unsigned int ConvertToMilliSeconds(Duration t) {
      return std::chrono::duration_cast<milliseconds>(t).count();
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

   void FakeSleep() {
      std::cout << "Fake Sleeping" << std::endl;
   }
}

TEST_F(AlarmClockTest, GetUsSleepTimeInUs) {
   int us = 123456;
   AlarmClock<microseconds> alerter(us);
   EXPECT_EQ(us, alerter.SleepTimeUs());
}

TEST_F(AlarmClockTest, GetUsSleepTimeInMs) {
   int us = 123456;
   AlarmClock<microseconds> alerter(us);
   EXPECT_EQ(ConvertToMilliSeconds(microseconds(us)), alerter.SleepTimeMs());
}

TEST_F(AlarmClockTest, GetMsSleepTimeInMs) {
   int ms = 123456;
   AlarmClock<milliseconds> alerter(ms);
   EXPECT_EQ(ms, alerter.SleepTimeMs());
}

TEST_F(AlarmClockTest, GetMsSleepTimeInUs) {
   int ms = 123456;
   AlarmClock<milliseconds> alerter(ms);
   EXPECT_EQ(ConvertToMicroSeconds(milliseconds(ms)), alerter.SleepTimeUs());
}

TEST_F(AlarmClockTest, GetSecSleepTimeInUs) {
   int sec = 1;
   AlarmClock<seconds> alerter(sec);
   EXPECT_EQ(ConvertToMicroSeconds(seconds(sec)), alerter.SleepTimeUs());
}

TEST_F(AlarmClockTest, GetSecSleepTimeInMs) {
   int sec = 1;
   AlarmClock<seconds> alerter(sec);
   EXPECT_EQ(ConvertToMilliSeconds(seconds(sec)), alerter.SleepTimeMs());
}

TEST_F(AlarmClockTest, microsecondsLessThan500ms) {
   int us = 900;
   StopWatch testTimer;
   AlarmClock<microseconds> alerter(us, &FakeSleep);
   WaitForAlarmClockToExpire(alerter);
   int totalTime = testTimer.ElapsedUs();
   auto maxTime = us + GetTimingLeeway(microseconds(us));
   EXPECT_TRUE(us <= totalTime) << "AlarmClock didn't sleep for long enough. Slept for: " << totalTime << " us, should be longer than " << us;
   EXPECT_TRUE(totalTime <= maxTime) << "AlarmClock took too long to expire. Took " << totalTime << " us. Should be less than " << maxTime;
   std::cout << "Timeout was set for " << us << " us. Actually slept for " << totalTime << " us. Max timeout: " << maxTime << std::endl;
}

// TEST_F(AlarmClockTest, microsecondsGreaterThan500ms) {
//    int us = 600000;
//    StopWatch testTimer;
//    AlarmClock<microseconds> alerter(us);
//    WaitForAlarmClockToExpire(alerter);
//    int totalTime = testTimer.ElapsedUs();
//    auto maxTime = us + GetTimingLeeway(microseconds(us));
//    EXPECT_TRUE(us <= totalTime) << "AlarmClock didn't sleep for long enough. Slept for: " << totalTime << " us, should be longer than " << us;
//    EXPECT_TRUE(totalTime <= maxTime) << "AlarmClock took too long to expire. Took " << totalTime << " us. Should be less than " << maxTime;
//    std::cout << "Timeout was set for " << us << " us. Actually slept for " << totalTime << " us. Max timeout: " << maxTime << std::endl;
// }

// TEST_F(AlarmClockTest, weirdNumberOfMicroseconds) {
//    int us = 724509;
//    StopWatch testTimer;
//    AlarmClock<microseconds> alerter(us);
//    WaitForAlarmClockToExpire(alerter);
//    int totalTime = testTimer.ElapsedUs();
//    auto maxTime = us + GetTimingLeeway(microseconds(us));
//    EXPECT_TRUE(us <= totalTime) << "AlarmClock didn't sleep for long enough. Slept for: " << totalTime << " us, should be longer than " << us;
//    EXPECT_TRUE(totalTime <= maxTime) << "AlarmClock took too long to expire. Took " << totalTime << " us. Should be less than " << maxTime;
//    std::cout << "Timeout was set for " << us << " us. Actually slept for " << totalTime << " us. Max timeout: " << maxTime << std::endl;
// }

// TEST_F(AlarmClockTest, millisecondsLessThan500) {
//    unsigned int ms = 100;
//    StopWatch testTimer;
//    AlarmClock<milliseconds> alerter(ms);
//    WaitForAlarmClockToExpire(alerter);
//    auto totalTime = testTimer.ElapsedUs();
//    auto msToMicro = ConvertToMicroSeconds(milliseconds(ms));
//    EXPECT_TRUE(msToMicro <= totalTime) << "AlarmClock didn't sleep for long enough. Slept for: " << totalTime << " us, should be longer than " << msToMicro;
//    auto maxTime = msToMicro + GetTimingLeeway(milliseconds(ms));
//    EXPECT_TRUE(totalTime <= maxTime) << "AlarmClock took too long to expire. Took " << totalTime << " us. Should be less than " << maxTime;
//    std::cout << "Timeout was set for " << msToMicro << " us. Actually slept for " << totalTime << " us. Max timeout: " << maxTime << std::endl;
// }

// TEST_F(AlarmClockTest, oneSecondInMilliseconds) {
//    unsigned int ms = 1000;
//    StopWatch testTimer;
//    AlarmClock<milliseconds> alerter(ms);
//    WaitForAlarmClockToExpire(alerter);
//    auto totalTime = testTimer.ElapsedUs();
//    auto msToMicro = ConvertToMicroSeconds(milliseconds(ms));
//    EXPECT_TRUE(msToMicro <= totalTime) << "AlarmClock didn't sleep for long enough. Slept for: " << totalTime << " us, should be longer than " << msToMicro;
//    auto maxTime = msToMicro + GetTimingLeeway(milliseconds(ms));
//    EXPECT_TRUE(totalTime <= maxTime) << "AlarmClock took too long to expire. Took " << totalTime << " us. Should be less than " << maxTime;
//    std::cout << "Timeout was set for " << msToMicro << " us. Actually slept for " << totalTime << " us. Max timeout: " << maxTime << std::endl;
// }

// TEST_F(AlarmClockTest, millisecondsNotDivisibleBy500) {
//    unsigned int ms = 1300;
//    StopWatch testTimer;
//    AlarmClock<milliseconds> alerter(ms);
//    WaitForAlarmClockToExpire(alerter);
//    auto totalTime = testTimer.ElapsedUs();
//    auto msToMicro = ConvertToMicroSeconds(milliseconds(ms));
//    EXPECT_TRUE(msToMicro <= totalTime) << "AlarmClock didn't sleep for long enough. Slept for: " << totalTime << " us, should be longer than " << msToMicro;
//    auto maxTime = msToMicro + GetTimingLeeway(milliseconds(ms));
//    EXPECT_TRUE(totalTime <= maxTime) << "AlarmClock took too long to expire. Took " << totalTime << " us. Should be less than " << maxTime;
//    std::cout << "Timeout was set for " << msToMicro << " us. Actually slept for " << totalTime << " us. Max timeout: " << maxTime << std::endl;
// }

// TEST_F(AlarmClockTest, secondsSimple) {
//    int sec = 1;
//    StopWatch testTimer;
//    AlarmClock<seconds> alerter(sec);
//    WaitForAlarmClockToExpire(alerter);
//    auto totalTime = testTimer.ElapsedUs();
//    auto secToMicro = ConvertToMicroSeconds(seconds(sec));
//    EXPECT_TRUE(secToMicro <= totalTime) << "AlarmClock didn't sleep for long enough. Slept for: " << totalTime << " sec, should be longer than " << secToMicro;
//    auto maxTime = secToMicro + GetTimingLeeway(seconds(sec));
//    EXPECT_TRUE(totalTime <= maxTime) << "AlarmClock took too long to expire. Took " << totalTime << " sec. Should be less than " << maxTime;
//    std::cout << "Timeout was set for " << secToMicro << " us. Actually slept for " << totalTime << " us. Max timeout: " << maxTime << std::endl;
// }

// TEST_F(AlarmClockTest, LongTimeout_ImmediatelyDestructed) {
//    int sec = 60;
//    StopWatch testTimer;
//    std::unique_ptr<AlarmClock<seconds>> acPtr(new AlarmClock<seconds>(sec));
//    std::this_thread::sleep_for(seconds(1));
//    EXPECT_EQ(ConvertToMilliSeconds(seconds(sec)), acPtr->SleepTimeMs());
//    EXPECT_FALSE(acPtr->Expired());
//    acPtr.reset();
//    EXPECT_TRUE(testTimer.ElapsedMs() < 10000);
// }

// TEST_F(AlarmClockTest, milliseconds_ResetAfterExpired) {
//    // First run
//    int ms = 750;
//    StopWatch testTimer;
//    AlarmClock<milliseconds> alerter(ms);
//    WaitForAlarmClockToExpire(alerter);
//    auto totalTime = testTimer.ElapsedUs();
//    auto msToMicro = ConvertToMicroSeconds(milliseconds(ms));
//    EXPECT_TRUE(msToMicro <= totalTime) << "AlarmClock didn't sleep for long enough. Slept for: " << totalTime << " sec, should be longer than " << msToMicro;
//    auto maxTime = msToMicro + GetTimingLeeway(milliseconds(ms));
//    EXPECT_TRUE(totalTime <= maxTime) << "AlarmClock took too long to expire. Took " << totalTime << " sec. Should be less than " << maxTime;
//    std::cout << "Timeout was set for " << msToMicro << " us. Actually slept for " << totalTime << " us. Max timeout: " << maxTime << std::endl;
   
//    // Reset after AlarmClock has expired
//    auto secondStartTime = testTimer.ElapsedUs();
//    alerter.Reset();
//    WaitForAlarmClockToExpire(alerter);
//    auto totalTime2 = testTimer.ElapsedUs() - secondStartTime;
//    EXPECT_TRUE(msToMicro <= totalTime2) << "AlarmClock didn't sleep for long enough. Slept for: " << totalTime2 << " sec, should be longer than " << msToMicro;
//    EXPECT_TRUE(totalTime2 <= maxTime) << "AlarmClock took too long to expire. Took " << totalTime2 << " sec. Should be less than " << maxTime;
//    std::cout << "Timeout (after reset) was set for " << msToMicro << " us. Actually slept for " << totalTime2 << " us. Max timeout: " << maxTime << std::endl;
// }

// TEST_F(AlarmClockTest, milliseconds_ResetBeforeExpired) {
//    // First run
//    int ms = 750;
//    auto msToMicro = ConvertToMicroSeconds(milliseconds(ms));
//    StopWatch testTimer;
//    AlarmClock<milliseconds> alerter(ms);
//    std::this_thread::sleep_for(milliseconds(200));

//    // Reset the AlarmClock before it expires
//    alerter.Reset();
//    auto timeToReset = testTimer.ElapsedUs();
//    std::cout << "AlarmClock set for " << msToMicro << " us, only slept for " << timeToReset << " before being reset." <<  std::endl;
//    EXPECT_TRUE(timeToReset < msToMicro) << "AlarmClock slept for full amount of time although it was rest before it had expired.";

//    // Let second run expire
//    WaitForAlarmClockToExpire(alerter);
//    auto totalTime = testTimer.ElapsedUs() - timeToReset;
//    EXPECT_TRUE(msToMicro <= totalTime) << "AlarmClock didn't sleep for long enough. Slept for: " << totalTime << " sec, should be longer than " << msToMicro;
//    auto maxTime = msToMicro + GetTimingLeeway(milliseconds(ms));
//    EXPECT_TRUE(totalTime <= maxTime) << "AlarmClock took too long to expire. Took " << totalTime << " sec. Should be less than " << maxTime;
//    std::cout << "Timeout (after reset) was set for " << msToMicro << " us. Actually slept for " << totalTime << " us. Max timeout: " << maxTime << std::endl;
   
// }