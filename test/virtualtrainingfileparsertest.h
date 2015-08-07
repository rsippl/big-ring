#ifndef VIRTUALTRAININGFILEPARSERTEST_H
#define VIRTUALTRAININGFILEPARSERTEST_H

#include <QObject>

class VirtualTrainingFileParserTest : public QObject
{
    Q_OBJECT
public:
    explicit VirtualTrainingFileParserTest(QObject *parent = 0);
private slots:
    void testWithBavellaFile();

};

#endif // VIRTUALTRAININGFILEPARSERTEST_H
