#include <qfile.h>

#include "adcthread.h"

ADCThread::ADCThread(QString adcType, int numChannels, QObject *parent)
	: QThread(parent)
{
	m_adcType = adcType;
	m_numChannels = numChannels;
	m_cancel = false;
	m_pollDelay = 1000 / DEFAULT_POLL_HZ;
}

ADCThread::~ADCThread()
{
	if (!isFinished()) {
		cancel();
		wait(10 * m_pollDelay);
	}
}

void ADCThread::cancel()
{
	const QMutexLocker locker(&m_mutex);
	m_cancel = true;
}

void ADCThread::setPollRate(int hz)
{
	const QMutexLocker locker(&m_mutex);

	if (hz < 1 || hz > 1000)
		return;

	m_pollDelay = 1000 / hz;
}

bool ADCThread::isCancelled() const
{
	const QMutexLocker locker(&m_mutex);
	return m_cancel;
}

void ADCThread::run()
{
	bool error = false;

	if (m_adcType != "mcp3008" && m_adcType != "ads1115") {
		emit adcMsg("Unknown ADC type: " + m_adcType);
		return;
	}

	if (m_numChannels < 1 || m_numChannels > 8) {
		emit adcMsg("Invalid channel count: " + QString::number(m_numChannels));
		return;
	}

	for (int i = 0; i < m_numChannels; i++)
		m_data.append(-1);

	emit adcMsg("ADCThread started");

	while (!isCancelled()) {
		if (!pollADC()) {
			error = true;
			break;
		}
		
		msleep(m_pollDelay);
	}

	if (!error)
		emit adcMsg("ADCThread finished");
}

QString ADCThread::readPath(int ch)
{
	if (m_adcType == "mcp3008")
		return "/sys/bus/iio/devices/iio:device0/in_voltage" + QString::number(ch) + "_raw";
	else
		return "/sys/class/hwmon/hwmon0/device/in" + QString::number(ch + 4) + "_input";
}

#ifdef Q_OS_WIN
bool ADCThread::pollADC()
{
	for (int i = 0; i < m_data.count(); i++)
		m_data[i]++;

	emit adcData(m_data);

	return true;
}
#else
bool ADCThread::pollADC()
{
	bool ok;
	char buff[16];
	bool change = false;

	for (int i = 0; i < m_data.count(); i++) {
		QFile file(readPath(i));

		if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
			file.close();
			emit adcMsg("Error opening file for ch " + QString::number(i) + " - Aborting read thread");
			return false;
		}

		memset(buff, 0, sizeof(buff));

		if (file.read(buff, sizeof(buff) - 1) < 0) {
			file.close();
			emit adcMsg("Error reading ch " + QString::number(i) + " value - Aborting read thread");
			return false;
		}

		int val = QString(buff).toInt(&ok);

		if (!ok) {
			file.close();
			emit adcMsg("Error converting ch " + QString::number(i) + " to an integer - Aborting read thread");
			return false;
		}

		file.close();

		if (m_data.at(i) != val) {
			m_data[i] = val;
			change = true;
		}
	}

	if (change)
		emit adcData(m_data);
	
	return true;
}
#endif
