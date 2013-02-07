/*
 * Copyright (c) 2009 Mark Rages
 * Copyright (c) 2011 Mark Liversedge (liversedge@gmail.com)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

//------------------------------------------------------------------------
// This code has been created by folding the ANT.cpp source with
// the Quarqd source provided by Mark Rages and the Serial device
// code from Computrainer.cpp
//------------------------------------------------------------------------

#include "ANT.h"
#include "ANTMessage.h"

#include "antdevicefinder.h"
#include "antdevice.h"
#ifdef Q_OS_WIN
#include "usbexpressantdevice.h"
#else
#include "unixserialusbant.h"
#endif

#include <QByteArray>
#include <QtDebug>

/* Control status */
#define ANT_RUNNING  0x01
#define ANT_PAUSED   0x02

namespace {
// network key
const unsigned char networkKey[8] = { 0xB9, 0xA5, 0x21, 0xFB, 0xBD, 0x72, 0xC3, 0x45 };
}
// supported sensor types
const ant_sensor_type_t ANT::ant_sensor_types[] = {
	{ ANTChannel::CHANNEL_TYPE_UNUSED, 0, 0, 0, 0, "Unused", '?', "" },
	{ ANTChannel::CHANNEL_TYPE_HR, ANT_SPORT_HR_PERIOD, ANT_SPORT_HR_TYPE,
	  ANT_SPORT_FREQUENCY, ANT_SPORT_NETWORK_NUMBER, "Heartrate", 'h', ":images/IconHR.png" },
	{ ANTChannel::CHANNEL_TYPE_POWER, ANT_SPORT_POWER_PERIOD, ANT_SPORT_POWER_TYPE,
	  ANT_SPORT_FREQUENCY, ANT_SPORT_NETWORK_NUMBER, "Power", 'p', ":images/IconPower.png" },
	{ ANTChannel::CHANNEL_TYPE_SPEED, ANT_SPORT_SPEED_PERIOD, ANT_SPORT_SPEED_TYPE,
	  ANT_SPORT_FREQUENCY, ANT_SPORT_NETWORK_NUMBER, "Speed", 's', ":images/IconSpeed.png" },
	{ ANTChannel::CHANNEL_TYPE_CADENCE, ANT_SPORT_CADENCE_PERIOD, ANT_SPORT_CADENCE_TYPE,
	  ANT_SPORT_FREQUENCY, ANT_SPORT_NETWORK_NUMBER, "Cadence", 'c', ":images/IconCadence.png" },
	{ ANTChannel::CHANNEL_TYPE_SandC, ANT_SPORT_SandC_PERIOD, ANT_SPORT_SandC_TYPE,
	  ANT_SPORT_FREQUENCY, ANT_SPORT_NETWORK_NUMBER, "Speed + Cadence", 'd', ":images/IconCadence.png" },

	// We comment out these Quarq specials -- they appear to be experimental
	//                                        but kept in the code incase they re-appear :)
	#if 0
	{ ANTChannel::CHANNEL_TYPE_QUARQ, ANT_QUARQ_PERIOD, ANT_QUARQ_TYPE,
	  ANT_QUARQ_FREQUENCY, DEFAULT_NETWORK_NUMBER, "Quarq Channel", 'Q', ":images/IconPower.png" },
	{ ANTChannel::CHANNEL_TYPE_FAST_QUARQ, ANT_FAST_QUARQ_PERIOD, ANT_FAST_QUARQ_TYPE,
	  ANT_FAST_QUARQ_FREQUENCY, DEFAULT_NETWORK_NUMBER, "Fast Quarq", 'q', ":images/IconPower.png" },
	{ ANTChannel::CHANNEL_TYPE_FAST_QUARQ_NEW, ANT_FAST_QUARQ_PERIOD, ANT_FAST_QUARQ_TYPE_WAS,
	  ANT_FAST_QUARQ_FREQUENCY, DEFAULT_NETWORK_NUMBER, "Fast Quarq New", 'n', ":images/IconPower.png" },
	#endif

	{ ANTChannel::CHANNEL_TYPE_GUARD, 0, 0, 0, 0, "", '\0', "" }
};

//
// The ANT class is a worker thread, reading/writing to a local
// Garmin ANT+ serial device. It maintains local state and telemetry.
// It is controlled by an ANTController, which starts/stops and will
// request telemetry and send commands to assign channels etc
//
// ANTController sits between the RealtimeWindow and the ANT worker
// thread and is part of the GC architecture NOT related to the
// hardware controller.
//
ANT::ANT(QObject *parent): QObject(parent),
	_antDeviceFinder(new indoorcycling::AntDeviceFinder),
	antDevice(NULL)
{
	powerchannels=0;

	// state machine
	state = ST_WAIT_FOR_SYNC;
	length = bytes = 0;
	checksum = ANT_SYNC_BYTE;

	// setup the channels
	for (int i=0; i<ANT_MAX_CHANNELS; i++) {

		// create the channel
		antChannel[i] = new ANTChannel(i, this);

		// connect up its signals
		connect(antChannel[i], SIGNAL(channelInfo(int,int,int)), this, SLOT(channelInfo(int,int,int)));
		connect(antChannel[i], SIGNAL(dropInfo(int,int,int)), this, SLOT(dropInfo(int,int,int)));
		connect(antChannel[i], SIGNAL(lostInfo(int)), this, SLOT(lostInfo(int)));
		connect(antChannel[i], SIGNAL(staleInfo(int)), this, SLOT(staleInfo(int)));
		connect(antChannel[i], SIGNAL(searchTimeout(int)), this, SLOT(slotSearchTimeout(int)));
		connect(antChannel[i], SIGNAL(searchComplete(int)), this, SLOT(slotSearchComplete(int)));

		connect(antChannel[i], SIGNAL(heartRateMeasured(quint8)), SIGNAL(heartRateMeasured(quint8)));
		connect(antChannel[i], SIGNAL(powerMeasured(float)), SIGNAL(powerMeasured(float)));
		connect(antChannel[i], SIGNAL(cadenceMeasured(float)), SIGNAL(cadenceMeasured(float)));
	}

	channels = 0;
}

ANT::~ANT()
{
	if (antDevice)
		antDevice->deleteLater();
}

double ANT::channelValue2(int channel)
{
	return antChannel[channel]->channelValue2();
}

bool ANT::isDevicePresent()
{
	bool present = (_antDeviceFinder->findAntDevice() != indoorcycling::ANT_DEVICE_NONE);
	if (!present)
		qDebug() << "no ANT+ stick found.";
	return present;
#ifdef Q_OS_WIN
//	return UsbExpressAntDevice::isDevicePresent();
#else
//	return UnixSerialUsbAnt::isDevicePresent();
#endif
}
double ANT::channelValue(int channel)
{
	return antChannel[channel]->channelValue();
}

/*======================================================================
 * Main thread functions; start, stop etc
 *====================================================================*/

void ANT::initialize()
{
	powerchannels = 0;


	if (!isDevicePresent()) {
		emit initializationFailed();
		return;
	}
	channels = 0;

	for (int i=0; i<ANT_MAX_CHANNELS; i++) antChannel[i]->init();

	state = ST_WAIT_FOR_SYNC;
	length = bytes = 0;
	checksum = ANT_SYNC_BYTE;

#ifdef Q_OS_WIN
	antDevice = new UsbExpressAntDevice;
#else
	antDevice = new UnixSerialUsbAnt;
#endif
	if (antDevice->isValid()) {
		channels = 4;
	} else {
		emit initializationFailed();
		return;
	}
	qDebug() << "valid";
	antlog.setFileName("antlog.bin");
	antlog.open(QIODevice::WriteOnly | QIODevice::Truncate);

	sendMessage(ANTMessage::resetSystem());
	sendMessage(ANTMessage::setNetworkKey(1, networkKey));

	// pair with specified devices on next available channel
	if (antIDs.count()) {

		foreach(QString antid, antIDs) {

			if (antid.length()) {
				unsigned char c = antid.at(antid.length()-1).toLatin1();
				int ch_type = interpretSuffix(c);
				int device_number = antid.mid(0, antid.length()-1).toInt();

				addDevice(device_number, ch_type, -1);
			}
		}

	} else {
		// not configured, just pair with whatever you can find
		addDevice(0, ANTChannel::CHANNEL_TYPE_SandC, 0);
		addDevice(0, ANTChannel::CHANNEL_TYPE_POWER, 1);
		addDevice(0, ANTChannel::CHANNEL_TYPE_CADENCE, 2);
		addDevice(0, ANTChannel::CHANNEL_TYPE_HR, 3);
	}
	emit initializationSucceeded();
}

void ANT::readCycle()
{
	bool bytesRead = false;

	while (true) {
		QByteArray bytes = rawRead();
		if (bytes.isEmpty())
			break;

		bytesRead = true;
		for (int i = 0; i < bytes.size(); ++i) {
			receiveByte((unsigned char) bytes.at(i));
		}
	}

	if (!bytesRead)
		return;

	// do we have a channel to search / stop
	if (!channelQueue.isEmpty()) {
		setChannelAtom x = channelQueue.dequeue();
		if (x.device_number == -1) antChannel[x.channel]->close(); // unassign
		else addDevice(x.device_number, x.channel_type, x.channel); // assign
	}
}

/*======================================================================
 * Channel management
 *====================================================================*/

// returns 1 for success, 0 for fail.
int
ANT::addDevice(int device_number, int device_type, int channel_number)
{
	// if we're given a channel number, then use that one
	if (channel_number>-1) {
		//antChannel[channel_number]->close();
		antChannel[channel_number]->open(device_number, device_type);
		return 1;
	}

	// if we already have the device, then return.
	// but only if the device number is given since
	// we may choose to scan for multiple devices
	// on separate channels (e.g. 0p on channel 0
	// and 0p on channel 1
	if (device_number != 0) {
		for (int i=0; i<channels; i++) {
			if (((antChannel[i]->channel_type & 0xf ) == device_type) &&
					(antChannel[i]->device_number == device_number)) {
				// send the channel found...
				//XXX antChannel[i]->channelInfo();
				return 1;
			}
		}
	}

	// look for an unused channel and use on that one
	for (int i=0; i<channels; i++) {
		if (antChannel[i]->channel_type == ANTChannel::CHANNEL_TYPE_UNUSED) {

			//antChannel[i]->close();
			antChannel[i]->open(device_number, device_type);

			// this is an alternate channel for power
			if (device_type == ANTChannel::CHANNEL_TYPE_POWER) {

				// if we are not the first power channel then set to update
				// the alternate power channel
				if (powerchannels) antChannel[i]->setAlt(true);

				// increment the number of power channels
				powerchannels++;
			}
			return 1;
		}
	}

	// there are no unused channels.  fail.
	return 0;
}

// returns 1 for successfully removed, 0 for none found.
int
ANT::removeDevice(int device_number, int channel_type)
{
	int i;

	for (i=0; i<channels; i++) {
		ANTChannel *ac = antChannel[i];

		if ((ac->channel_type == channel_type) && (ac->device_number == device_number)) {

			if ((ac->control_channel!=ac) && ac->control_channel)
				removeDevice(device_number, ac->control_channel->channel_type);

			ac->close();
			ac->channel_type=ANTChannel::CHANNEL_TYPE_UNUSED;
			ac->device_number=0;
			ac->setId();
			return 1;
		}
	}

	// device not found.
	return 0;
}

ANTChannel *
ANT::findDevice(int device_number, int channel_type)
{

	int i;

	for (i=0; i<channels; i++) {
		if (((antChannel[i]->channel_type) == channel_type) &&
				(antChannel[i]->device_number==device_number)) {
			return antChannel[i];
		}
	}

	// device not found.
	return NULL;
}

int
ANT::startWaitingSearch()
{
	int i;

	// are any fast searches in progress?  if so, then bail
	for (i=0; i<channels; i++) {
		if (antChannel[i]->channel_type_flags & CHANNEL_TYPE_QUICK_SEARCH) {
			return 0;
		}
	}

	// start the first slow search
	for (i=0; i<channels; i++) {
		if (antChannel[i]->channel_type_flags & CHANNEL_TYPE_WAITING) {
			antChannel[i]->channel_type_flags &= ~CHANNEL_TYPE_WAITING;
			sendMessage(ANTMessage::unassignChannel(i));
			return 1;
		}
	}

	return 0;
}

void
ANT::associateControlChannels() {

	// first, unassociate all control channels
	for (int i=0; i<channels; i++) antChannel[i]->control_channel=NULL;

	// then, associate cinqos:
	//    new cinqos get their own selves for control
	//    old cinqos, look for an open control channel
	//       if found and open, associate
	//       elif found and not open yet, nop
	//       elif not found, open one
	for (int i=0; i<channels; i++) {
		ANTChannel *ac=antChannel[i];

		switch (ac->channel_type) {
		case ANTChannel::CHANNEL_TYPE_POWER:
			if (ac->is_cinqo) {
				if (ac->is_old_cinqo) {
					ANTChannel *my_ant_channel;

					my_ant_channel=findDevice(ac->device_number, ANTChannel::CHANNEL_TYPE_QUARQ);
					if (!my_ant_channel) my_ant_channel=findDevice(ac->device_number, ANTChannel::CHANNEL_TYPE_FAST_QUARQ);
					if (!my_ant_channel) my_ant_channel=findDevice(ac->device_number, ANTChannel::CHANNEL_TYPE_FAST_QUARQ_NEW);

					if (my_ant_channel) {
						if (my_ant_channel->isSearching()) {
							// ignore if searching
						} else {
							ac->control_channel=my_ant_channel;
							ac->sendCinqoSuccess();
						}
					} else { // no ant channel, let's start one
						addDevice(ac->device_number, ANTChannel::CHANNEL_TYPE_QUARQ, -1);
					}
				} else { // new cinqo
					ac->control_channel=ac;
					ac->sendCinqoSuccess();
				}
			} // is_cinqo
			break;

		case ANTChannel::CHANNEL_TYPE_FAST_QUARQ:
		case ANTChannel::CHANNEL_TYPE_FAST_QUARQ_NEW:
		case ANTChannel::CHANNEL_TYPE_QUARQ:
			ac->is_cinqo=1;
			ac->control_channel=ac;
			break;
		default:
			;
		} // channel_type case
	} // for-loop
}

void
ANT::channelInfo(int channel, int device_number, int device_id)
{
	QString description(deviceTypeDescription(device_id));
	QString typeCode(device_id);

	qDebug()<<"found device number"<<device_number<<"type"<<device_id<<"on channel"<<channel
		   << "is a "<< description << "with code"<< typeCode;

	emit foundDevice(channel, device_number, device_id, description, typeCode);
}

void
ANT::dropInfo(int channel, int drops, int received)    // we dropped a message
{
	double reliability = 100.00f - (100.00f * double(drops) / double(received));
	//qDebug()<<"Channel"<<channel<<"reliability is"<< (int)(reliability)<<"%";
	emit signalStrength(channel, reliability);
	return;
}

void
ANT::lostInfo(int number)    // we lost the connection
{
	if (number < 0 || number >= channels) return; // ignore out of bound

	emit lostDevice(number);
	//qDebug()<<"lost info for channel"<<number;
}

void
ANT::staleInfo(int number)   // info is now stale - set to zero
{
	if (number < 0 || number >= channels) return; // ignore out of bound

	//qDebug()<<"stale info for channel"<<number;
}

void
ANT::slotSearchTimeout(int number) // search timed out
{
	if (number < 0 || number >= channels) return; // ignore out of bound

	emit searchTimeout(number);
	//qDebug()<<"search timeout on channel"<<number;
}

void
ANT::slotSearchComplete(int number) // search completed successfully
{
	if (number < 0 || number >= channels) return; // ignore out of bound

	emit searchComplete(number);
	//qDebug()<<"search completed on channel"<<number;
}

/*----------------------------------------------------------------------
 * Message I/O
 *--------------------------------------------------------------------*/
void
ANT::sendMessage(ANTMessage m) {

	QByteArray bytes((const char*) m.data, m.length);
	rawWrite(bytes);

	// this padding is important, for some reason XXX find out why?

	static const char padding[5] = { '\0', '\0', '\0', '\0', '\0' };
	QByteArray paddingBytes(padding, 5);
	rawWrite(paddingBytes);
}

void
ANT::receiveByte(unsigned char byte) {

	switch (state) {
	case ST_WAIT_FOR_SYNC:
		if (byte == ANT_SYNC_BYTE) {
			state = ST_GET_LENGTH;
			checksum = ANT_SYNC_BYTE;
			rxMessage[0] = byte;
		}
		break;

	case ST_GET_LENGTH:
		if ((byte == 0) || (byte > ANT_MAX_LENGTH)) {
			state = ST_WAIT_FOR_SYNC;
		}
		else {
			rxMessage[ANT_OFFSET_LENGTH] = byte;
			checksum ^= byte;
			length = byte;
			bytes = 0;
			state = ST_GET_MESSAGE_ID;
		}
		break;

	case ST_GET_MESSAGE_ID:
		rxMessage[ANT_OFFSET_ID] = byte;
		checksum ^= byte;
		state = ST_GET_DATA;
		break;

	case ST_GET_DATA:
		rxMessage[ANT_OFFSET_DATA + bytes] = byte;
		checksum ^= byte;
		if (++bytes >= length){
			state = ST_VALIDATE_PACKET;
		}
		break;

	case ST_VALIDATE_PACKET:
		if (checksum == byte){
			processMessage();
		}
		state = ST_WAIT_FOR_SYNC;
		break;
	}
}


//
// Pass inbound message to channel for handling
//
void
ANT::handleChannelEvent(void) {
	int channel = rxMessage[ANT_OFFSET_DATA] & 0x7;
	if(channel >= 0 && channel < channels) {

		// handle a channel event here!
		antChannel[channel]->receiveMessage(rxMessage);
	}
}

void
ANT::processMessage(void) {

	ANTMessage m(this, rxMessage); // for debug!

	//fprintf(stderr, "<< receive: ");
	//for(int i=0; i<m.length+3; i++) fprintf(stderr, "%02x ", m.data[i]);
	//fprintf(stderr, "\n");

	QDataStream out(&antlog);
	for (int i=0; i<ANT_MAX_MESSAGE_SIZE; i++)
		out<<rxMessage[i];


	switch (rxMessage[ANT_OFFSET_ID]) {
	case ANT_ACK_DATA:
	case ANT_BROADCAST_DATA:
	case ANT_CHANNEL_STATUS:
	case ANT_CHANNEL_ID:
	case ANT_BURST_DATA:
		handleChannelEvent();
		break;

	case ANT_CHANNEL_EVENT:
		switch (rxMessage[ANT_OFFSET_MESSAGE_CODE]) {
		case EVENT_TRANSFER_TX_FAILED:
			//XXX remember last message ... ANT_SendAckMessage();
			break;
		case EVENT_TRANSFER_TX_COMPLETED:
			// fall through
		default:
			handleChannelEvent();
		}
		break;

	case ANT_VERSION:
		break;

	case ANT_CAPABILITIES:
		break;

	case ANT_SERIAL_NUMBER:
		break;

	default:
		break;
	}
}

int ANT::rawWrite(QByteArray &bytes) // unix!!
{
	return antDevice->writeBytes(bytes);
}

QByteArray ANT::rawRead()
{
	return antDevice->readBytes();
}

// convert 'p' 'c' etc into ANT values for device type
int ANT::interpretSuffix(char c)
{
	const ant_sensor_type_t *st=ant_sensor_types;

	do {
		if (st->suffix==c) return st->type;
	} while (++st, st->type != ANTChannel::CHANNEL_TYPE_GUARD);

	return -1;
}

// convert ANT value to 'p' 'c' values // XXX this and below are named wrong, legacy from quarqd code.
char ANT::deviceIdCode(int type)
{
	const ant_sensor_type_t *st=ant_sensor_types;

	do {
		if (st->type==type) return st->suffix;
	} while (++st, st->type != ANTChannel::CHANNEL_TYPE_GUARD);
	return '-';
}

// convert ANT value to 'p' 'c' values
char ANT::deviceTypeCode(int type)
{
	const ant_sensor_type_t *st=ant_sensor_types;

	do {
		if (st->device_id==type) return st->suffix;
	} while (++st, st->type != ANTChannel::CHANNEL_TYPE_GUARD);
	return '-';
}

// convert ANT value to human string
const char * ANT::deviceTypeDescription(int type)
{
	const ant_sensor_type_t *st=ant_sensor_types;

	do {
		if (st->device_id==type) return st->descriptive_name;
	} while (++st, st->type != ANTChannel::CHANNEL_TYPE_GUARD);
	return "Unknown device type";
}

