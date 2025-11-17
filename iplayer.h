#ifndef IPLAYER_H
#define IPLAYER_H

#include <QObject>
#include "piece.h"

class IPlayer : public QObject
{
    Q_OBJECT
public:
    explicit IPlayer(Player player, QObject *parent = nullptr)
        : QObject(parent), m_player(player) {}
    virtual ~IPlayer() {}

    virtual void requestMove(const QString& gameState) = 0;
    Player getPlayer() const { return m_player; }

signals:
    void moveReady(const QString& moveNotation);

protected:
    Player m_player;
};

#endif
