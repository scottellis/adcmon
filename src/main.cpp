#include "adcmon.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	ADCMon w;

	qRegisterMetaType< QList<int> >("QList<int>");

	w.show();
	return a.exec();
}
