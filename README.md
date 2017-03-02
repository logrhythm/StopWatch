
AlarmClock
==========
This gives the option to query whether a specified time period has expired. The utility is made to have a very low CPU clock cycle cost compared using StopWatch or std::chrono.
This utility strives to be as accurate as possible, with the assumption that low CPU usage and cheap, CPU wise,  API calls are more important than absolute accuracy in terms of some extra microseconds of expiration overhead. 

Common expiration time overhead ranged from 50 - 150 microseconds on the current testing platform when the Alarm clock was checked for expiration in a continous loop. (which normally is not a good usage pattern)

The API usage can be found in: [[AlarmClock.h]](https://github.com/LogRhythm/StopWatch/blob/master/src/AlarmClock.h) and in [[AlarmClockTest.cpp]](https://github.com/LogRhythm/StopWatch/blob/master/test/AlarmClockTest.cpp)


StopWatch
=========

A timer class in C++ which wraps the std::chrono API, with the purpose of retrieving elapsed time since creation of the object or since the timing was restarted. 
Measurements are available in microseconds, milliseconds and seconds


ThreadSafeStopWatch
===================

A replacement for `thread_local StopWatch` on platforms that do not have the `thread_local` keyword accessible. It implements the `thread_local`  through a `lazy-barber` map access solution.
If `thread_local` is available on your platform then  `thread_local StopWatch` is likely a better choice than using the`ThreadSafeStopWatch`