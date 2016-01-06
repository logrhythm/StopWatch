/* 
 * File:   AlarmClockSpeedTest.cpp
 * Author: Amanda Carbonari
 * Created: January 6, 2015 9:00am
 *
 * WARNING: This only measures CPU time
 */

#include <ctime>
#include <iostream>
#include <AlarmClock.h>
#include <boost/timer/timer.hpp>
using namespace std;
typedef std::chrono::microseconds microseconds;
typedef std::chrono::milliseconds milliseconds;
typedef std::chrono::seconds seconds;


template<typename T> void WaitForAlarmClockToExpire(AlarmClock<T>& alerter) {
   while(!alerter.Expired());
}

int main(int, const char**) {
   clock_t start;
   clock_t end;
   unsigned int us = 389;

   cout << "Creating Alarm Clock" << endl;
   AlarmClock<microseconds> alerter(us);
   // Give some time for the countdown to start
   boost::this_thread::sleep_for(boost::chrono::microseconds(20));
   cout << "Starting clock and resetting" << endl;
   //start = clock();
   {
      boost::timer::auto_cpu_timer t;
      alerter.Reset();
   }
   //end = clock();
   //clock_t reset_time = (clock() - start) / (double) (CLOCKS_PER_SEC);
   //cout << "Start: " << start << ", End: " << end << endl;
   cout << "Waiting for the clock to expire" << endl;
   WaitForAlarmClockToExpire(alerter);

   //cout << "Time: " << reset_time << " us" << endl;
}