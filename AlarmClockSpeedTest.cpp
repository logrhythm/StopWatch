/* 
 * File:   AlarmClockSpeedTest.cpp
 * Author: Amanda Carbonari
 * Created: January 6, 2015 9:00am
 *
 * WARNING: This measurement will include sleep time (for overall)
 */

#include <ctime>
#include <iostream>
#include <AlarmClock.h>
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
   high_resolution_clock::time_point start_overall = high_resolution_clock::now();
   cout << "Creating Alarm Clock" << endl;
   //AlarmClock<microseconds> alerter(us);
   AlarmClock<T> alerter(sleep_time);
   cout << "Starting clock and resetting" << endl;
   high_resolution_clock::time_point start = high_resolution_clock::now();
   alerter.Reset();
   high_resolution_clock::time_point end = high_resolution_clock::now();
   auto reset_time = duration_cast<microseconds>(end - start).count();
   cout << "Waiting for the clock to expire" << endl;
   WaitForAlarmClockToExpire(alerter);
   high_resolution_clock::time_point end_overall = high_resolution_clock::now();
   auto overall_time = duration_cast<microseconds>(end_overall - start_overall).count();
   cout << "Reset Time: " << reset_time << " us" << endl;
   cout << "Overall Time: " << overall_time << " us" << endl;
   auto slept_time = alerter.SleptTime();
   if (slept_time == -1) {
      slept_time = alerter.SleepTimeUs();
   }

   auto overhead = overall_time + reset_time - slept_time;
   cout << "Overhead Time: " << overhead << " us" << endl;
}

int main(int, const char**) {
   unsigned int us = 1000;
   unsigned int ms = 3;
   unsigned int s = 1;

   cout << "Testing microseconds" << endl;
   testReset<microseconds>(us);
   cout << "Testing milliseconds" << endl;
   testReset<milliseconds>(ms);
   cout << "Testing seconds" << endl;
   testReset<seconds>(s);

}
