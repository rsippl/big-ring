# 1.7.1

- Add ANT+ messages log file. Useful for debugging ANT+ behaviour.
- Add configuration option for aspect ratio of videos. Fixes #64.
- Always show distance markers in profile. Fixes #65.

# 1.6.1

- Fix for GPS interpolation. Fixes #60.

# 1.6.0

- Checks for new release on start up.
- Adds GPS positions to tcx files if rlv file contains gps information.
- ANT+ FE-C: always ask trainer for capabilities before sending 
  configuration.
- ANT+: when search for sensor fails, retry search after a second.
- When using imperial units, user and bike weight can be configured
  in pounds (lbs).
- Make folder where TCX files are saved user configurable.
- Add Google Analytics tracking to determine how often the program
  is used.

# 1.5.0

- Fix OpenGL code. A faulty fragment shader was used that worked, but
not on newer hardware.
- Restoring window size after a ride should work better. Not perfect yet
on Windows unfortunately.
- Simplify directory structure. Makes build easier to understand.

# 1.4.0

- Add the ability to save rides to TCX.
- Remove "Unfinished runs". This functionality was broken.
- Stability fixes for ANT+. 
  - Retry search when sensor is not found when doing a ride.
  - Close all ANT+ channels when closing "Add Sensor Dialog"


# 1.3.0

- ALPHA support for ANT+ FE-C trainers.
- Ride GUI: Dials are at the bottom now.
- Ride GUI: Only show profile for the current course.
- RLV import: Use a cache to speed up import of xml and gpx files.

# 1.2.0

- Average power values over 0, 1, 3 or 10 seconds. #42 

# 1.1.0

- A course can now be finished. A ride is not recorded yet. #11
- Support information messages displayed at certain distances during a course. #26
   - infoboxes (CmdList) on Tacx.
   - <informations> tag in VirtualTraining.
- Show "distance to go" instead of "distance travelled".
- Highlight current course in profile during ride.
- Only emit cadence values from power meters that support cadence values. #41
- Elite Qubo Fluid and Elite Turbo Muin virtual power added.

