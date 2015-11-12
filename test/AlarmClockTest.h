/* 
 * File:   AlarmClockTest.h
 * Author: Craig Cogdill
 * Created: October 15, 2015 10:45am
 */

#pragma once
#include "AlarmClock.h"
#include "gtest/gtest.h"

class AlarmClockTest : public ::testing::Test {
public:

   AlarmClockTest() {};

   static unsigned int mFakeSleepUs;

protected:

   virtual void SetUp() { AlarmClockTest::mFakeSleepUs = 0; };

   virtual void TearDown() { AlarmClockTest::mFakeSleepUs = 0; };
};
