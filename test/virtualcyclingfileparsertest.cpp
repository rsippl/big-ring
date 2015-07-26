#include "virtualcyclingfileparsertest.h"

#include "virtualcyclingfileparser.h"
#include <QtCore/QFile>
#include <QtTest/QTest>

namespace {
const QFileInfo BAVELLA_VIDEO_FILE = QFileInfo("/media/video/RLV/FR_Bavella.avi");
const QFileInfo DOBEL_VIDEO_FILE = QFileInfo("/media/video/RLV/DE_Dobel.avi");

QList<QFileInfo> videoFiles = { DOBEL_VIDEO_FILE, BAVELLA_VIDEO_FILE };
}
VirtualCyclingFileParserTest::VirtualCyclingFileParserTest(QObject *parent):
    QObject(parent)
{
    // empty
}

void VirtualCyclingFileParserTest::testWithBavellaFile()
{
    QFile f(":///resources/FR_Bavella-utf8.xml");
    QVERIFY(f.exists());

    indoorcycling::VirtualCyclingFileParser parser(videoFiles);
    std::unique_ptr<RealLifeVideo> rlv = parser.parseVirtualCyclingFile(f);

    QVERIFY(rlv.get());
    QCOMPARE(rlv->name(), QString("Col de Bavella"));
    QCOMPARE(rlv->videoInformation().videoFilename(), QString("/media/video/RLV/FR_Bavella.avi"));
    QCOMPARE(rlv->videoInformation().frameRate(), 25.0f);

    Profile profile = rlv->profile();
    QCOMPARE(profile.altitudeForDistance(0), 12.5033845f);
    QCOMPARE(profile.altitudeForDistance(38440), 750.67939f);
    QCOMPARE(profile.slopeForDistance(38432), -0.098766f);

    QList<Course> courses = rlv->courses();
    QCOMPARE(courses.size(), 3);
    QCOMPARE(courses[1].name(), QString("Erstes Teilstück"));
    QCOMPARE(courses[1].start(), 0.0f);
    QCOMPARE(courses[1].end(), 20280.0f);

}
