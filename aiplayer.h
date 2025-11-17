#ifndef AIPLAYER_H
#define AIPLAYER_H

#include "abstractplayer.h"
#include <QTimer>

class AIPlayer : public IPlayer
{
    Q_OBJECT
public:
    explicit AIPlayer(Player player, QObject *parent = nullptr);

    void requestMove(const QString& gameState) override;

private slots:
    void onThinkComplete();
};

#endif
