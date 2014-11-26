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

* Works on Linux. On Windows, video is not smooth for HD movies. For some reason, loading a video frame as an OpenGL texture is much too slow on Windows. I'm probably doing something wrong.
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
* [Qt](http://qt.digia.com) 5.2. This library is used throughout the program.
* [gstreamer](http://gstreamer.freedesktop.org/) for video decoding and displaying.
* [Qt-Gstreamer](http://gstreamer.freedesktop.org/modules/qt-gstreamer.html) for coupling gstreamer to Qt. Mainly uses Qt Gstreamer qt5glvideosink for efficiently displaying video frames.
* [G++](http://gcc.gnu.org), the compiler.
* [libusbx](http://libusbx.org), for handling of the usb device. On Ubuntu, installing libusb-dev will install libusbx.
* An OpenGL capable system for the graphics.
* [Google Test](https://code.google.com/p/googletest/), OPTIONAL, for test code.

Limitations
-----------

* No recording of data.
* No way to finish a course. You can start at different points, but it will only finish when you stop (not pause) or when the video reaches the end.

Building
--------

1. Create a build directory, for instance next to the source directory.
2. Run qmake <source directory> from the build directory.
3. make
4. the indoorcycling1 executable will be located in the bin/ directory inside the build directory.

File/Device Permissions
-----------------------

To be able to send data to the ANT+ USB sticks, the user needs permissions. On Linux systems with *udev*, this can established by putting the following rules in the udev configuration. 

	SUBSYSTEMS=="usb" ATTR{idVendor}=="0fcf", ATTR{idProduct}=="1004", MODE="666"
	SUBSYSTEMS=="usb" ATTR{idVendor}=="0fcf", ATTR{idProduct}=="1008", MODE="666"

On Ubuntu, a superuser can put a file with the lines above in `/etc/udev/rules.d`.

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

