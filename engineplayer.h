#ifndef ENGINEPLAYER_H
#define ENGINEPLAYER_H
#include "abstractplayer.h"
#include <QProcess>

class EnginePlayer : public IPlayer
{
    Q_OBJECT
public:
    explicit EnginePlayer(Player player, const QString& enginePath, QObject *parent = nullptr);
    ~EnginePlayer();
    void requestMove(const QString& gameState) override;

private slots:
    void onReadyReadStandardOutput();
    void onReadyReadStandardError();
    void onEngineError(QProcess::ProcessError error);
    void onEngineStateChanged(QProcess::ProcessState newState);

signals:
    void engineLog(const QString& message);

private:
    QProcess* m_process;
    QString m_enginePath;
};
#endif
