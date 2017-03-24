#.rst:
# FindStopWatch
# -------
#
# Find libStopWatch
# StopWatch: wrapper around the std::chrono API for easy time expiration retrieval. 
# AlarmClock: Low CPU clock cycle cost to retrieve if a time period has expired. 
#
# Result variables
# ^^^^^^^^^^^^^^^^
#
# This module will set the following variables in your project:
#
# ``STOPWATCH_INCLUDE_DIRS``
#   where to find StopWatch.h AlarmClock.h et.al
#
# ``STOPWATCH_LIBRARIES``
#   the libraries to link against to use libStopWatch.
#   that includes libStopWatch library files.
#
# ``STOPWATCH_FOUND``
#   If false, do not try to use StopWatch.
#
include(FindPackageHandleStandardArgs)
find_path(STOPWATCH_INCLUDE_DIR StopWatch.h AlarmClock.h ThreadSafeStopWatch.h)
find_library(STOPWATCH_LIBRARY
            NAMES libStopWatch StopWatch)

find_package_handle_standard_args(STOPWATCH  DEFAULT_MSG
            STOPWATCH_INCLUDE_DIR STOPWATCH_LIBRARY)
            
mark_as_advanced(STOPWATCH_INCLUDE_DIR STOPWATCH_LIBRARY)
set(STOPWATCH_LIBRARIES ${STOPWATCH_LIBRARY})
set(STOPWATCH_INCLUDE_DIRS ${STOPWATCH_INCLUDE_DIR})