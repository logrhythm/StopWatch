/* 
 * File:   AlarmClockSpeedTest.cpp
 * Author: Amanda Carbonari
 * Created: January 6, 2015 9:00am
 */

#include <ctime>
#include <iostream>
#include <AlarmClock.h.old> // To test other implementations, change this to #include<header file> or #inlcude<AlarmClock.h.old> for example
#include <chrono>
#include <thread>
using namespace std;
using namespace std::chrono;
typedef std::chrono::microseconds microseconds;
typedef std::chrono::milliseconds milliseconds;
typedef std::chrono::seconds seconds;


template<typename T> void WaitForAlarmClockToExpire(AlarmClock<T>& alerter) {
   while(!alerter.Expired());
}

template <typename T> void testReset(unsigned int sleep_time) {
   // Start overall measurements
   high_resolution_clock::time_point start_overall = high_resolution_clock::now();
   cout << "Creating Alarm Clock" << endl;
   AlarmClock<T> alerter(sleep_time);
   cout << "Starting clock and resetting" << endl;

   // Sleep before restart, this can be changed to see the overhead for
   // different reset periods
   std::this_thread::sleep_for(microseconds(sleep_time/2));

   // Start the reset measurement
   high_resolution_clock::time_point start = high_resolution_clock::now();

   alerter.Reset();
   
   // Get the ending reset measurment
   high_resolution_clock::time_point end = high_resolution_clock::now();

   // Calculate the reset time
   auto reset_time = duration_cast<microseconds>(end - start).count();
   
   cout << "Waiting for the clock to expire" << endl;
   // Make sure the clock expires correctly after reset
   WaitForAlarmClockToExpire(alerter);

   // Get the ending overall time
   high_resolution_clock::time_point end_overall = high_resolution_clock::now();

   // Calculate the overall time
   auto overall_time = duration_cast<microseconds>(end_overall - start_overall).count();
   cout << "Results:" << endl;
   cout << "\tReset Time: " << reset_time << " us" << endl;
   cout << "\tOverall Time: " << overall_time << " us" << endl;
   
   // The overhead is calculated differently between Craig's and my code.
   // Since I have code to time my sleep, I added a method that gets the 
   // actual amount of sleep time. 
   // Craig's code does not have this and I didn't want to change it's overall
   // functionality to measure performance, so I just added the same method
   // and have it return -1. If it does, then it just uses the amount of time
   // the Alarm Clock should have slept for a measurement. 
   auto slept_time = alerter.SleptTime();
   if (slept_time == -1) {
      slept_time = alerter.SleepTimeUs();
   }

   // Overhead is just the overall time minus the slept time. 
   auto overhead = overall_time - slept_time;
   cout << "\tOverhead Time: " << overhead << " us" << endl;
}

int main(int, const char**) {
   // The tester could take in different values as arguments, for now I have
   // some hard coded. 
   unsigned int us = 400;
   unsigned int ms = 3;
   unsigned int s = 1;

   cout << "---------------------------- Testing " << us << " microseconds ----------------------------" << endl;
   testReset<microseconds>(us);
   cout << "---------------------------- Testing " << ms << " milliseconds ----------------------------" << endl;
   testReset<milliseconds>(ms);
   cout << "---------------------------- Testing " << s << " seconds ----------------------------" << endl;
   testReset<seconds>(s);

}
