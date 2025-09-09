#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QMutex>

class Logger : public QObject
{
    Q_OBJECT

public:
    explicit Logger(const QString &filePath, QObject *parent = nullptr);
    ~Logger();

    void write(const QString &message);

private:
    QFile m_logFile;
    QTextStream m_textStream;
    QMutex m_mutex;
};

#endif // LOGGER_H
