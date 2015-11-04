/* 
 * File:   AlarmClockTest.cpp
 * Author: Craig Cogdill
 * Created: October 15, 2015 10:45am
 */

#include "AlarmClockTest.h"
#include "MockAlarmClock.h"
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
   void WaitForBackgroundThreadToStart(MockAlarmClock<T>& alerter) {
      while (!alerter.ThreadIsAlive());
   }
   
   template<typename T>
   void WaitForAlarmClockToExpire(MockAlarmClock<T>& alerter) {
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
}

TEST_F(AlarmClockTest, GetUsSleepTimeInUs) {
   int us = 123456;
   MockAlarmClock<microseconds> alerter(us);
   EXPECT_EQ(us, alerter.SleepTimeUs());
}

TEST_F(AlarmClockTest, GetUsSleepTimeInMs) {
   int us = 123456;
   MockAlarmClock<microseconds> alerter(us);
   EXPECT_EQ(ConvertToMilliSeconds(microseconds(us)), alerter.SleepTimeMs());
}

TEST_F(AlarmClockTest, GetMsSleepTimeInMs) {
   int ms = 123456;
   MockAlarmClock<milliseconds> alerter(ms);
   EXPECT_EQ(ms, alerter.SleepTimeMs());
}

TEST_F(AlarmClockTest, GetMsSleepTimeInUs) {
   int ms = 123456;
   MockAlarmClock<milliseconds> alerter(ms);
   EXPECT_EQ(ConvertToMicroSeconds(milliseconds(ms)), alerter.SleepTimeUs());
}

TEST_F(AlarmClockTest, GetSecSleepTimeInUs) {
   int sec = 1;
   MockAlarmClock<seconds> alerter(sec);
   EXPECT_EQ(ConvertToMicroSeconds(seconds(sec)), alerter.SleepTimeUs());
}

TEST_F(AlarmClockTest, GetSecSleepTimeInMs) {
   int sec = 1;
   MockAlarmClock<seconds> alerter(sec);
   EXPECT_EQ(ConvertToMilliSeconds(seconds(sec)), alerter.SleepTimeMs());
}

TEST_F(AlarmClockTest, LongTimeout_ImmediatelyDestructed) {
   int sec = 60;
   StopWatch testTimer;
   std::unique_ptr<MockAlarmClock<seconds>> acPtr(new MockAlarmClock<seconds>(sec));
   EXPECT_EQ(ConvertToMilliSeconds(seconds(sec)), acPtr->SleepTimeMs());
   EXPECT_FALSE(acPtr->Expired());
   acPtr.reset();
   EXPECT_TRUE(testTimer.ElapsedMs() < 1000);
}

TEST_F(AlarmClockTest, milliseconds_Reset) {
   // First run
   int ms = 750;
   MockAlarmClock<milliseconds> alerter(ms);
   WaitForBackgroundThreadToStart(alerter);
   EXPECT_EQ(1, alerter.GetNumberOfSleeps());

   alerter.Reset();
   EXPECT_EQ(0, alerter.GetNumberOfSleeps());

   WaitForAlarmClockToExpire(alerter);
   EXPECT_EQ(1, alerter.GetNumberOfSleeps());
}

TEST_F(AlarmClockTest, secondsSimple) {
   int sec = 1;
   MockAlarmClock<seconds> alerter(1);
   EXPECT_FALSE(alerter.Expired());
   WaitForAlarmClockToExpire(alerter);
   EXPECT_TRUE(alerter.Expired());
   EXPECT_EQ(1, alerter.GetNumberOfSleeps());
}

TEST_F(AlarmClockTest, millisecondsSimple) {
   int ms = 2200;
   MockAlarmClock<milliseconds> alerter(ms);
   EXPECT_FALSE(alerter.Expired());
   WaitForAlarmClockToExpire(alerter);
   EXPECT_TRUE(alerter.Expired());
   EXPECT_EQ(4, alerter.GetNumberOfSleeps());
}

TEST_F(AlarmClockTest, microsecondsSimple) {
   int us = 2200000;
   MockAlarmClock<microseconds> alerter(us);
   EXPECT_FALSE(alerter.Expired());
   WaitForAlarmClockToExpire(alerter);
   EXPECT_TRUE(alerter.Expired());
   EXPECT_EQ(4, alerter.GetNumberOfSleeps());
}