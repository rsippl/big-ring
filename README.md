Big Ring Indoor Video Cycling
=============================

![screenshot](https://cloud.githubusercontent.com/assets/420742/6061426/0d0dc792-ad48-11e4-93b7-739a7560a0cf.jpg)

Big Ring Indoor Video Cycling can be used in combination with a bicycle, indoor trainer and [ANT+](http://www.thisisant.com) power, cadence or heart rate sensors.
An example video, with a slightly older GUI can be seen [here on Youtube](https://www.youtube.com/watch?v=bYnc8lVDvuY).

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

* Works on Linux & Windows 
* Get information from ANT+ Sensors
    - Power meters
    - Cadence sensors (including cadence from power meters)
    - Speed sensors
    - Combined Cadence & Speed Sensors
    - Heart rate sensors
* Show current values for:
    - Power, directly measured from Power meter or derived from speed and power curve of trainer
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
and [O-Synce](http://www.o-synce.com/en/) and [Garmin](http://www.garmin.com/garmin/cms/site/us) heart rate straps.
* An [ANT+ USB stick](https://buy.garmin.com/shop/shop.do?pID=10997).
I have the USB 1 variant from Garmin and a USB 2 from Suunto, the MoveStick Mini. The Suunto MoveStick Mini is internally just a Garmin USB 2 Stick, so that one should also work. 
* [Qt](http://qt.digia.com) 5.3. This library is used throughout the program.
* [libav](https://libav.org/) for video decoding and displaying.
* [G++](http://gcc.gnu.org), the compiler.
* [libusb](http://www.libusb.org/) for handling of the usb device. On Ubuntu, installing libusb-1.0-0-dev will install libusb. Version 1.0 is used on Linux. The standard version that is used on Windows
for ANT+ is libusb-win32, which is based on libusb-0.1. Because of this, internally the API of libusb-0.1 is used. On Linux the source of [libusb-compat](http://www.libusb.org/wiki/libusb-compat-0.1) is included to make a bridge from the libusb-0.1 API to libusb-1.0.
* An OpenGL capable system for the graphics.

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

	SUBSYSTEM=="tty", ATTRS{idVendor}=="0fcf", ATTRS{idProduct}=="1004", SYMLINK+="garmin-usb1", MODE="666"
	SUBSYSTEMS=="usb" ATTR{idVendor}=="0fcf", ATTR{idProduct}=="1008", MODE="666"

On Ubuntu, a superuser can put a file with the lines above in `/etc/udev/rules.d`.

Usage
-----

Run `indoorcycling2\bin\big-ring` program. The program will start and try to find your videos. If no video folder has been configured yet, the program will ask you to configure it. The files will be parsed and when ready, the list of videos will be populated. Using the preferences window, the user can configure the ANT+ sensors. Choose a video, and a course. 

Roadmap
-------

* Support OS/X.
* Record data to files (FIT/Powertap CSV/.. ?)
* Improve user management.
* Many more.

License
-------

Big Ring Indoor Video Cycling is distributed under the GNU General Public License, version 3 or later. See LICENSE.txt for details.

