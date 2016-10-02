#include <qsettings.h>
#include <qdir.h>
#include <qboxlayout.h>
#include <qformlayout.h>

#include "adcmon.h"

ADCMon::ADCMon(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	readConfig();

	layoutWindow();
	layoutStatusBar();

	m_adcThread = NULL;

	connect(m_exit, SIGNAL(triggered()), this, SLOT(close()));
	connect(m_start, SIGNAL(triggered()), this, SLOT(onStart()));
	connect(m_stop, SIGNAL(triggered()), this, SLOT(onStop()));

	ui.actionStop->setEnabled(false);
	ui.actionStart->setEnabled(true);

	restoreWindowPos();
}

void ADCMon::closeEvent(QCloseEvent *)
{
	if (m_adcThread) {
		m_adcThread->cancel();
		m_adcThread->wait(1000);
		delete m_adcThread;
	}

	saveConfig();
}

void ADCMon::onStart()
{
	m_stop->setEnabled(true);
	m_start->setEnabled(false);

	if (!m_adcThread) {
		m_adcThread = new ADCThread(m_adcType, m_numChannels, this);
		connect(m_adcThread, SIGNAL(adcMsg(QString)), this, SLOT(adcMsg(QString)));
		connect(m_adcThread, SIGNAL(adcData(QList<int>)), this, SLOT(adcData(QList<int>)));
		m_adcThread->start();
	}
}

void ADCMon::onStop()
{
	m_stop->setEnabled(false);

	if (m_adcThread) {
		m_adcThread->cancel();
		m_adcThread->wait(1000);
		disconnect(m_adcThread, SIGNAL(adcMsg(QString)), this, SLOT(adcMsg(QString)));
		disconnect(m_adcThread, SIGNAL(adcData(QList<int>)), this, SLOT(adcData(QList<int>)));
		delete m_adcThread;
		m_adcThread = nullptr;
	};

	m_start->setEnabled(true);
}

void ADCMon::adcMsg(QString msg)
{
	m_adcStatusLbl->setText(msg);
}

void ADCMon::adcData(QList<int> data)
{
	for (int i = 0; i < data.count() && i < m_chValue.count(); i++)
		m_chValue[i]->setText(QString::number(data.at(i)));
}

void ADCMon::layoutWindow()
{
	QVBoxLayout *mainLayout = new QVBoxLayout;
	
	for (int i = 0; i < m_numChannels; i++) {
		QLineEdit *edit = new QLineEdit("-1");
		edit->setMaximumWidth(80);
		edit->setAlignment(Qt::AlignRight);
		edit->setReadOnly(true);
		m_chValue.append(edit);

		QFormLayout *formLayout = new QFormLayout;
		formLayout->addRow("Ch" + QString::number(i), edit);
		mainLayout->addLayout(formLayout);
	}

	m_exit = new QAction("Exit");
	m_start = new QAction("Start");
	m_stop = new QAction("Stop");

	ui.mainToolBar->setFloatable(false);
	ui.mainToolBar->setMovable(false);
	ui.mainToolBar->addAction(m_exit);
	ui.mainToolBar->addSeparator();
	ui.mainToolBar->addAction(m_start);
	ui.mainToolBar->addAction(m_stop);

	centralWidget()->setLayout(mainLayout);
}

void ADCMon::layoutStatusBar()
{
	m_adcTypeLbl = new QLabel("ADC: " + m_adcType);
	m_adcTypeLbl->setFrameStyle(QFrame::Panel);
	m_adcTypeLbl->setFrameShadow(QFrame::Sunken);
	ui.statusBar->addWidget(m_adcTypeLbl, 0);

	m_adcStatusLbl = new QLabel;
	m_adcStatusLbl->setFrameStyle(QFrame::Panel);
	m_adcStatusLbl->setFrameShadow(QFrame::Sunken);
	ui.statusBar->addWidget(m_adcStatusLbl, 1);
}

void ADCMon::saveConfig()
{
	QString path = QDir::homePath();

	if (path == "/")
		path = "/home/root/";

	QSettings *settings = new QSettings(path + "/adcmon.ini", QSettings::IniFormat);

	if (!settings)
		return;

#ifdef Q_OS_WIN
	settings->beginGroup("Window");
	settings->setValue("Geometry", saveGeometry());
	settings->setValue("State", saveState());
	settings->endGroup();
#endif

	settings->beginGroup("ADC");
	settings->setValue("Type", m_adcType);
	settings->endGroup();

	delete settings;
}

void ADCMon::readConfig()
{
	QString path = QDir::homePath();

	if (path == "/")
		path = "/home/root/";

	QSettings *settings = new QSettings(path + "/adcmon.ini", QSettings::IniFormat);

	if (!settings)
		return;

	settings->beginGroup("ADC");
	m_adcType = settings->value("Type", "mcp3008").toString();
	settings->endGroup();

	if (m_adcType == "mcp3008")
		m_numChannels = 8;
	else if (m_adcType == "ads1115")
		m_numChannels = 4;
	else
		m_numChannels = 1;

	delete settings;
}

void ADCMon::restoreWindowPos()
{
#ifdef Q_OS_WIN
	QString path = QDir::homePath();

	QSettings *settings = new QSettings(path + "/adcmon.ini", QSettings::IniFormat);

	if (!settings)
		return;

	settings->beginGroup("Window");
	restoreGeometry(settings->value("Geometry").toByteArray());
	restoreState(settings->value("State").toByteArray());
	settings->endGroup();

	delete settings;
#endif
}