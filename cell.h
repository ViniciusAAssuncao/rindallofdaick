#ifndef CELL_H
#define CELL_H

#include "piece.h"

class Cell
{
public:
    Cell() : resources(0), controller(Player::Player1), isControlled(false) {}

    int getResources() const { return resources; }
    void setResources(int res) { resources = res; }
    void addResource() { resources++; }

    int getDefense() const { return resources; }

    Player getController() const { return controller; }
    void setController(Player player) { controller = player; isControlled = true; }

    bool isUnderControl() const { return isControlled; }
    void removeControl() { isControlled = false; }

private:
    int resources;
    Player controller;
    bool isControlled;
};

#endif
