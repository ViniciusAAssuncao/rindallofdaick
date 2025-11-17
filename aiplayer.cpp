#include "aiplayer.h"
#include <QDebug>

AIPlayer::AIPlayer(Player player, QObject *parent)
    : IPlayer(player, parent)
{
}

void AIPlayer::requestMove(const QString& gameState)
{
    Q_UNUSED(gameState);
    qDebug() << "AIPlayer: Received request for move. Thinking...";

    QTimer::singleShot(1000, this, &AIPlayer::onThinkComplete);
}

void AIPlayer::onThinkComplete()
{
    qDebug() << "AIPlayer: Thinking complete. Sending dummy move.";

    if(m_player == Player::Player2)
    {
        emit moveReady("FA11A10");
    }
    else
    {
        emit moveReady("FA2A3");
    }
}
