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
using namespace std;
typedef std::chrono::microseconds microseconds;
typedef std::chrono::milliseconds milliseconds;
typedef std::chrono::seconds seconds;


template<typename T> void WaitForAlarmClockToExpire(AlarmClock<T>& alerter) {
	while(!alerter.Expired());
}

int main(int, const char**) {
 	clock_t start;
 	unsigned int ms = 389;

 	AlarmClock<milliseconds> alerter(ms);

 	start = clock();
 	alerter.Reset();
 	clock_t reset_time = (clock() - start) / (double) (CLOCKS_PER_SEC/1000);

 	WaitForAlarmClockToExpire(alerter);

 	cout << "Time: " << reset_time << " ms" << endl;
}