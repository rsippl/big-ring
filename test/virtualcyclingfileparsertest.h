#ifndef VIRTUALCYCLINGFILEPARSERTEST_H
#define VIRTUALCYCLINGFILEPARSERTEST_H

#include <QObject>

class VirtualCyclingFileParserTest : public QObject
{
    Q_OBJECT
public:
    explicit VirtualCyclingFileParserTest(QObject *parent = 0);
private slots:
    void testWithBavellaFile();

};

#endif // VIRTUALCYCLINGFILEPARSERTEST_H
