#include "humanplayer.h"

HumanPlayer::HumanPlayer(Player player, QObject *parent)
    : IPlayer(player, parent)
{
}

void HumanPlayer::requestMove(const QString& gameState)
{
    Q_UNUSED(gameState);
}

void HumanPlayer::processHumanMove(const QString& moveNotation)
{
    emit moveReady(moveNotation);
}
