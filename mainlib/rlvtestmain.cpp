#include "rlvtester.h"
#include <QCoreApplication>
int main(int argc, char** argv) {
	QCoreApplication app(argc, argv);

	QString rootDir(argv[1]);
	RlvTester tester(rootDir);

	QObject::connect(&tester, SIGNAL(finished()), &app, SLOT(quit()));
	app.exec();
}
