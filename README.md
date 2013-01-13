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

* Get information from ANT+ Sensors
    - Power meters
    - Heart rate meters
* Show current values for:
    - Power
    - Cadence
    - Heart Rate

Dependencies
------------

Indoor Cycling uses the following components to work:

* ANT+ Sensors. I use a [Power2Max](http://www.power2max.com/) power meter
and a [Garmin](http://www.garmin.com/garmin/cms/site/us)
heart rate strap.
* An [ANT+ USB stick](https://buy.garmin.com/shop/shop.do?pID=10997).
I have the USB 1.0 variant. There is also
a USB 2.0 variant. I do not know if this last variant works with Quarqd.
* [Quarqd](http://opensource.quarq.us/quarqd/), a daemon program written
by the [Quarq](http://www.quarq.com/),
makers of the Quarq Cinqo power meter. Quarqd connects to the
ANT+ USB stick to connect to the sensors.
* A [Python](http://www.python.org) runtime, as Indoor Cycling is written
in Python.
* [Python Twisted](http://twistedmatrix.com/trac/) library, used for the
connection to quarqd.
* [PySide](http://www.pyside.org/), a Python Qt binding, used for the GUI.


Limitations
-----------

* Only runs on systems supporting Quarqd. So to my knowledge, that means just
Linux and Max OS X.
* Quarqd needs to be started before Indoor Cycling is started.
* If sensors time out, no connection is made to them again.
* No recording of data.

Building
--------

A binary version of Quarqd for Linux i386, x86_64 and OS X can be downloaded
from the [Quarqd](http://opensource.quarq.us/quarqd/) site. Look inside the
build- directories for the binaries. An example configuration file is
distributed with Indoor Cycling, under the name `example_quarqd_config`.
Save this file under the name `.quarqd_config` in your home directory.

Usage
-----

* Start Quarqd
* Start Indoor Cycling, using the executable file `main.py`. A connection
to Quarqd will be made and the values from the sensors will be shown
immediately.

Roadmap
-------

* Before everything else, the tests need to be improved, to make it easier to
make changes to the software. Because it takes quite some time to do a manual
test (setup bike and trainer, hook up computer etc), the test suite should
find as many errors and problems as possible. This saves a lot of time.
* Reconnect to sensors when they're lost, for example from inactivity. #3
* Record data to files.
* Start Quarqd from within Indoor Cycling if it is not running yet.
* (Longer term): Replace Quarqd by an internal mechanism for connecting with
the ANT+ stick. For now, Quarqd is the easiest way to connect to the stick.
Replacing it will cost quite a lot of time.
* Be able to play Tacx real life video. The video should run at the correct
speed for the power that is produced and the (virtual) slope of the road.