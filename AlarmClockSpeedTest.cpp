/* 
 * File:   AlarmClockSpeedTest.cpp
 * Author: Amanda Carbonari
 * Created: January 6, 2015 9:00am
 */

#include <ctime>
#include <iostream>
#include <AlarmClock.h> // To test other implementations, change this to #include<header file> or #inlcude<AlarmClock.h.old> for example
#include <chrono>
#include <thread>
using namespace std;
using namespace std::chrono;
typedef std::chrono::microseconds microseconds;
typedef std::chrono::milliseconds milliseconds;
typedef std::chrono::seconds seconds;


template<typename T> int WaitForAlarmClockToExpire(AlarmClock<T>& alerter) {
   int i = 1;
   for(; !alerter.Expired(); ++i);
   return i;
}

void getSleepOverhead(unsigned int sleep_time) {
   high_resolution_clock::time_point start_overall = high_resolution_clock::now();
   AlarmClock<microseconds> alerter(sleep_time);
   int forLoopVal = WaitForAlarmClockToExpire(alerter);
   high_resolution_clock::time_point end_overall = high_resolution_clock::now();

   auto overall_time = duration_cast<microseconds>(end_overall - start_overall).count();
   int slept_time = alerter.SleepTimeUs();
   auto overhead = overall_time - slept_time;
   cout << "Results: " << endl;
   cout << "\tOverall Time: " << overall_time << " us" << endl;
   cout << "\tOverhead Time: " << overhead << " us" << endl;
   cout << "\tSleep Time: " << slept_time << " us" << endl;
   cout << "\tFor Loop Count: " << forLoopVal << endl;
}

void testReset(unsigned int sleep_time, int reset_portion=-1) {
   // Start overall measurements
   high_resolution_clock::time_point start_overall = high_resolution_clock::now();
   AlarmClock<microseconds> alerter(sleep_time);

   // Sleep before restart, this can be changed to see the overhead for
   // different reset periods
   auto wait_amount = sleep_time/reset_portion;
   if (reset_portion == -1) {
      wait_amount = 0;
   }
   std::this_thread::sleep_for(microseconds(wait_amount));

   // Start the reset measurement
   high_resolution_clock::time_point start = high_resolution_clock::now();

   alerter.Reset();
   
   // Get the ending reset measurment
   high_resolution_clock::time_point end = high_resolution_clock::now();

   // Calculate the reset time
   auto reset_time = duration_cast<microseconds>(end - start).count();
   
   // Make sure the clock expires correctly after reset
   WaitForAlarmClockToExpire(alerter);

   // Get the ending overall time
   high_resolution_clock::time_point end_overall = high_resolution_clock::now();

   // Calculate the overall time
   auto overall_time = duration_cast<microseconds>(end_overall - start_overall).count();
   int slept_time = alerter.SleepTimeUs();

   // Overhead is just the overall time minus the slept time and wait period.
   auto overhead = overall_time - slept_time - wait_amount;
   cout << "Results:" << endl;
   cout << "\tOverall Running Time: " << overall_time << " us" << endl;
   cout << "\tOverhead Time: " << overhead << " us" << endl;
   cout << "\tOverhead Breakdowns: " << endl;
   cout << "\t\tReset Time: " << reset_time << " us" << endl;
   cout << "\t\tSleeping Overhead: " << duration_cast<microseconds>(end_overall - end).count() - slept_time << " us" << endl;
   cout << "\tWait Before Restart Time: " << wait_amount << " us" << endl;
   cout << "\tTime Spent Sleeping: " << slept_time << " us" << endl;
}

int main(int, const char**) {
   // The tester could take in different values as arguments, for now I have
   // some hard coded. 
   microseconds us = microseconds(400);
   milliseconds ms = milliseconds(3);
   seconds s = seconds(1);

   for (int reset_time = 2; reset_time < 10; ++reset_time) {
      cout << "Reset @ 1/" << reset_time << endl;
      cout << "---------------------------- Testing " << us.count() << " microseconds ----------------------------" << endl;
      testReset(us.count(), reset_time);
      cout << "---------------------------- Testing " << ms.count() << " milliseconds ----------------------------" << endl;
      testReset(microseconds(ms).count(), reset_time);
      cout << "---------------------------- Testing " << s.count() << " seconds ----------------------------" << endl;
      testReset(microseconds(s).count(), reset_time);
   }

   cout << "Reset Immediately" << endl;
   cout << "---------------------------- Testing " << us.count() << " microseconds ----------------------------" << endl;
   testReset(us.count());
   cout << "---------------------------- Testing " << ms.count() << " milliseconds ----------------------------" << endl;
   testReset(microseconds(ms).count());
   cout << "---------------------------- Testing " << s.count() << " seconds ----------------------------" << endl;
   testReset(microseconds(s).count());

   cout << "No Reset and For Loop Test" << endl;
   cout << "---------------------------- Testing " << us.count() << " microseconds ----------------------------" << endl;
   getSleepOverhead(us.count());
   cout << "---------------------------- Testing " << ms.count() << " milliseconds ----------------------------" << endl;
   getSleepOverhead(microseconds(ms).count());
   cout << "---------------------------- Testing " << s.count() << " seconds ----------------------------" << endl;
   getSleepOverhead(microseconds(s).count());

}
