Indoor Cycling
==============

This program can be used in combination with a bicycle, indoor trainer
and [ANT+](http://www.thisisant.com) power, cadence or heart rate sensors.

Goal
----

The goal for this program is to be an almost complete replacement for the
"real life video" cycling programs that are offered by
[Tacx](http://www.tacx.com), [Elite](http://www.elite-it.com/) and
[CompuTrainer](http://www.racermateinc.com/computrainer.asp).
Of course, without a way to provide feedback to the back wheel,
this program can not give users the same experience as those programs.
However, it also prevents users from having to buy an expensive piece
of equipment.

Current Status
--------------

Indoor Cycling currently does the following:

* Works on Ubuntu Linux. It's a goal to support Windows and OS X,
  but I've done no work to support them yet.
* Get information from ANT+ Sensors
    - Power meters
    - Cadence meter (including cadence from power meters)
    - Heart rate meters
* Show current values for:
    - Power
    - Cadence
    - Heart Rate
* Play a Real Life Video with video frames mapped to the distance
  the cyclist has travelled. The distance is determined by calculating
  (approximating) the speed of the cyclist several times per second, 
  and changing the travelled distance according to the speed.

Dependencies
------------

Indoor Cycling uses the following components to work:

* ANT+ Sensors. I use a [Power2Max](http://www.power2max.com/) power meter
and a [Garmin](http://www.garmin.com/garmin/cms/site/us)
heart rate strap.
* An [ANT+ USB stick](https://buy.garmin.com/shop/shop.do?pID=10997).
I have the USB 1 variant from Garmin and a USB 2 from Suunto, the MoveStick Mini. The Suunto MoveStick Mini is internally just a Garmin USB 2 Stick, so that one should also work. 
* [Qt](http://qt.digia.com) 4.8. This library is used throughout the program.
* [Libav](http://libav.org) for video decoding.
* [CMake](http://www.cmake.org), for building.
* [G++](http://gcc.gnu.org), the compiler.
* [libusbx](http://libusbx.org), for handling of the usb device. On Ubuntu, installing libusb-dev will install libusbx.
* And OpenGL capable system for the graphics.

Limitations
-----------

* Only runs well on Linux. Video on Windows is not yet smooth, because loading video frames to OpenGL textures takes too long for large resolutions.
* No recording of data.
* Tacx videos have some problem with the position of the frames. Not all
  videos have the correct mapping from distance to frames. Will need to look
  into this.

Building
--------

1. Create a build directory, for instance next to the source directory.
2. Run cmake <source directory> from the build directory.
3. make
4. All executables should build.

Usage
-----

Run `indoorcycling` with the folder containing your real life video files (.rlv, .pgmf and .avi) as the argument. The program will start and try to find your videos. The files will be parsed and when ready, the list of videos will be populated. The program starts looking for ANT+ sensors right away. If you have no power meter, it cannot determine the cyclists speed. Restart the program with the word "robot" as the second parameter. This will give the cyclist a constant power of 300W.

Choose a video, and a course. Press play or press the space bar to play and pause. Use 'F' to go full screen, 'ESC' to go back to windowed mode.

Roadmap
-------

* Support Windows.
* Support OS/X.
* Record data to files (FIT/Powertap CSV/.. ?)
* Show elevation graph and current position.
* Improve user management.
* Many more.

