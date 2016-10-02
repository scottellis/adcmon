#ifndef ADCTHREAD_H
#define ADCTHREAD_H

#include <qthread.h>
#include <qmutex.h>
#include <qlist.h>

#define DEFAULT_POLL_HZ 10

class ADCThread : public QThread
{
	Q_OBJECT

public:
	explicit ADCThread(QString adcType, int numChannels, QObject *parent = nullptr);
	virtual ~ADCThread();

	void cancel();
	void setPollRate(int hz);

signals:
	void adcData(QList<int> data);
	void adcMsg(QString msg);

private:
	bool isCancelled() const;
	void run();
	bool pollADC();
	QString readPath(int ch);

	mutable QMutex m_mutex;
	bool m_cancel;
	int m_pollDelay;
	QString m_adcType;
	int m_numChannels;
	QList<int> m_data;
};

#endif /* ADCTHREAD_H */
