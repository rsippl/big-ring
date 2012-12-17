#include <QTest>
#include "reallivevideo.h"

class TestQString: public QObject
 {
	 Q_OBJECT
 private slots:
	void testProfileEntryDefault();
	void testProfileEntry();

	void testProfileDefault();
	void testProfile();

	void testVideoInformationDefault();
	void testVideoInformation();
};

void TestQString::testProfileEntryDefault()
{
	ProfileEntry entry;
	QCOMPARE(entry.distance(), 0.0f);
	QCOMPARE(entry.slope(), 0.0f);
}

void TestQString::testProfileEntry()
{
	ProfileEntry entry(1.23f, 8.23f);
	QCOMPARE(entry.distance(), 1.23f);
	QCOMPARE(entry.slope(), 8.23f);
}

void TestQString::testProfileDefault()
{
	Profile profile;

	QCOMPARE(profile.startAltitude(), 0.0f);
	QCOMPARE(profile.slopeForDistance(0.0), 0.0f);
	QCOMPARE(profile.slopeForDistance(10.0), 0.0f);
}

void TestQString::testProfile()
{
	QMap<float,ProfileEntry> entries;
	entries[0.0] = ProfileEntry(0.0, 1.0);
	entries[1.0] = ProfileEntry(1.0, 2.0);
	Profile profile(12, entries);

	QCOMPARE(profile.startAltitude(), 12.0f);

	QCOMPARE(profile.slopeForDistance(1.1), 2.0f);
	QCOMPARE(profile.slopeForDistance(0.5), 1.0f);

	QCOMPARE(profile.altitudeForDistance(0.0f), 12.0f);
	QCOMPARE(profile.altitudeForDistance(1.0f), 12.01f);
	QCOMPARE(profile.altitudeForDistance(2.0f), 12 + 0.01f + 0.02f);
	QCOMPARE(profile.altitudeForDistance(1.5f), 12 + 0.01f + 0.01f);
}

void TestQString::testVideoInformationDefault()
{
	VideoInformation info;
	QCOMPARE(0.0f, info.frameRate());
	QCOMPARE(info.videoFilename(), QString(""));
}

void TestQString::testVideoInformation()
{
	VideoInformation info("VideoFile", 29.97f);
	QCOMPARE(29.97f, info.frameRate());
	QCOMPARE(info.videoFilename(), QString("VideoFile"));
}

QTEST_MAIN(TestQString)
#include "reallivevideotest.moc"
