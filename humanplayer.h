#ifndef HUMANPLAYER_H
#define HUMANPLAYER_H

#include "abstractplayer.h"

class HumanPlayer : public IPlayer
{
    Q_OBJECT
public:
    explicit HumanPlayer(Player player, QObject *parent = nullptr);

    void requestMove(const QString& gameState) override;

public slots:
    void processHumanMove(const QString& moveNotation);
};

#endif
