#include "engineplayer.h"
#include <QDebug>
#include <QCoreApplication>
#include <QDir>

EnginePlayer::EnginePlayer(Player player, const QString& enginePath, QObject *parent)
    : IPlayer(player, parent), m_process(new QProcess(this)), m_enginePath(enginePath)
{
    connect(m_process, &QProcess::readyReadStandardOutput, this, &EnginePlayer::onReadyReadStandardOutput);
    connect(m_process, &QProcess::readyReadStandardError, this, &EnginePlayer::onReadyReadStandardError);
    connect(m_process, &QProcess::errorOccurred, this, &EnginePlayer::onEngineError);
    connect(m_process, &QProcess::stateChanged, this, &EnginePlayer::onEngineStateChanged);

    QStringList arguments;
    m_process->setProgram(m_enginePath);





    m_process->setWorkingDirectory(QCoreApplication::applicationDirPath());




    m_process->start(QProcess::Unbuffered | QProcess::ReadWrite);

    if (!m_process->waitForStarted(5000)) {
        qWarning() << "Engine failed to start:" << m_process->errorString();
        emit engineLog(QString("ERROR: Could not start engine at %1").arg(enginePath));
    } else {
        qDebug() << "Engine started successfully:" << enginePath;
    }
}

EnginePlayer::~EnginePlayer()
{
    if (m_process->state() != QProcess::NotRunning) {
        m_process->terminate();
        m_process->waitForFinished(3000);
    }
}

void EnginePlayer::requestMove(const QString& gameState)
{
    if (m_process->state() == QProcess::Running) {
        qDebug() << "EnginePlayer: Writing gamestate to engine stdin...";
        m_process->write(gameState.toUtf8());


        m_process->write("\nEND_STATE\n");



        if (!m_process->waitForBytesWritten(1000)) {
            qWarning() << "EnginePlayer: Timed out waiting for bytes to be written!";
        } else {
            qDebug() << "EnginePlayer: Gamestate write finished.";
        }
    } else {
        qWarning() << "Engine is not running. Cannot request move.";
    }
}

void EnginePlayer::onReadyReadStandardOutput()
{
    while (m_process->canReadLine()) {
        QString line = m_process->readLine().trimmed();
        if (!line.isEmpty()) {
            qDebug() << "EnginePlayer: Received move from engine:" << line;
            emit moveReady(line);
            break;
        }
    }
}


void EnginePlayer::onReadyReadStandardError()
{
    QByteArray errorData = m_process->readAllStandardError();
    qWarning() << "EnginePlayer: Received STDERR from engine:" << QString::fromLocal8Bit(errorData);
    emit engineLog(QString("ENGINE STDERR: %1").arg(QString::fromLocal8Bit(errorData)));
}

void EnginePlayer::onEngineError(QProcess::ProcessError error)
{
    qWarning() << "Engine error:" << error << m_process->errorString();
    emit engineLog(QString("ENGINE ERROR: %1").arg(m_process->errorString()));
}

void EnginePlayer::onEngineStateChanged(QProcess::ProcessState newState)
{
    qDebug() << "Engine state changed:" << newState;
}
