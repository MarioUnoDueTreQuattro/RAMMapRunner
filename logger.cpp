#include "Logger.h"
#include <QDateTime>

Logger::Logger(const QString &filePath, QObject *parent)
    : QObject(parent), m_logFile(filePath)
{
    if (m_logFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_textStream.setDevice(&m_logFile);
    }
}

Logger::~Logger()
{
    if (m_logFile.isOpen()) {
        m_logFile.close();
    }
}

void Logger::write(const QString &message)
{
    QMutexLocker locker(&m_mutex); // ensure thread-safety

    if (m_logFile.isOpen()) {
        QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        m_textStream << timestamp << " - " << message << "\n";
        m_textStream.flush();
    }
}
