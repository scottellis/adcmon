#ifndef ADCMON_H
#define ADCMON_H

#include <QtWidgets/QMainWindow>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qaction.h>

#include "ui_adcmon.h"

#include "adcthread.h"

class ADCMon : public QMainWindow
{
	Q_OBJECT

public:
	ADCMon(QWidget *parent = 0);

public slots:
	void onStart();
	void onStop();

	void adcMsg(QString msg);
	void adcData(QList<int> data);

protected:
	void closeEvent(QCloseEvent *);

private:
	void layoutWindow();
	void layoutStatusBar();
	void saveConfig();
	void readConfig();
	void restoreWindowPos();

	Ui::ADCMonClass ui;

	ADCThread *m_adcThread;

	QAction *m_start;
	QAction *m_stop;
	QAction *m_exit;

	QList<QLineEdit *> m_chValue;

	QLabel *m_adcTypeLbl;
	QLabel *m_adcStatusLbl;

	QString m_adcType;
	int m_numChannels;
};

#endif // ADCMON_H
