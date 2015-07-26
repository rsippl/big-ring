#include "virtualcyclingfileparsertest.h"

#include "rlvfileparser.h"
#include "virtualcyclingfileparser.h"
#include <QtCore/QFile>
#include <QtTest/QTest>

namespace {
const QFileInfo BAVELLA_VIDEO_FILE = QFileInfo("/media/video/RLV/FR_Bavella.avi");
const QFileInfo DOBEL_VIDEO_FILE = QFileInfo("/media/video/RLV/DE_Dobel.avi");
const QFileInfo BAVELLA_PGMF_FILE = QFileInfo(":///resources/FR_Bavella.pgmf");
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
    QFile fTacx(":///resources/FR_Bavella.rlv");

    RlvFileParser rlvFileParser({BAVELLA_PGMF_FILE}, videoFiles);

    indoorcycling::VirtualCyclingFileParser parser(videoFiles);
    std::unique_ptr<RealLifeVideo> rlv = parser.parseVirtualCyclingFile(f);
    RealLifeVideo tacxRlv = rlvFileParser.parseRlvFile(fTacx);

    QVERIFY(rlv.get());
    QCOMPARE(rlv->name(), QString("Col de Bavella"));
    QCOMPARE(rlv->videoInformation().videoFilename(), QString("/media/video/RLV/FR_Bavella.avi"));
    QCOMPARE(rlv->videoInformation().frameRate(), 25.0f);
    QCOMPARE(tacxRlv.videoInformation().frameRate(), 25.0f);

    Profile profile = rlv->profile();
    float startAltitude = 12.5033845f;
    QCOMPARE(profile.altitudeForDistance(0), startAltitude);
    QCOMPARE(tacxRlv.profile().altitudeForDistance(0), 0.0f);

    QCOMPARE(profile.altitudeForDistance(38440), 750.67939f);
    QCOMPARE(qRound(profile.slopeForDistance(38432) * 100), -10);
    QCOMPARE(qRound(tacxRlv.profile().slopeForDistance(38432) * 100), -10);

    QList<Course> courses = rlv->courses();
    QCOMPARE(courses.size(), 3);
    QCOMPARE(courses[1].name(), QString("Erstes Teilstück"));
    QCOMPARE(courses[1].start(), 0.0f);
    QCOMPARE(courses[1].end(), 20280.0f);

    QCOMPARE(rlv->frameForDistance(0.0f), 0u);
    QCOMPARE(tacxRlv.frameForDistance(0.0f), 0u);
    QCOMPARE(rlv->frameForDistance(332.8), 1000u);
    QCOMPARE(tacxRlv.frameForDistance(332.8), 1000u);
}
