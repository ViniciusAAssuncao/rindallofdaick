#ifndef CELL_H
#define CELL_H

#include "piece.h"

class Cell
{
public:
    Cell() : resources(0), resourceOwner(Player::Player1), hasResourceOwner(false),
        controller(Player::Player1), isControlled(false), bastionDefense(0) {}

    int getResources() const { return resources; }
    void setResources(int res) { resources = res; }
    void addResource(Player owner) {
        resources++;
        resourceOwner = owner;
        hasResourceOwner = true;
    }

    bool removeResource() {
        if (resources > 0) {
            resources--;
            if (resources == 0) {
                hasResourceOwner = false;
            }
            return true;
        }
        return false;
    }

    int getDefense(Player requestingPlayer) const {
        if (hasResourceOwner && resourceOwner == requestingPlayer) {
            return resources + bastionDefense;
        }
        return bastionDefense;
    }

    int getDefense() const {
        return resources + bastionDefense;
    }

    Player getResourceOwner() const { return resourceOwner; }
    bool hasOwner() const { return hasResourceOwner; }

    void setBastionDefense(int def) { bastionDefense = def; }
    int getBastionDefense() const { return bastionDefense; }

    Player getController() const { return controller; }
    void setController(Player player) { controller = player; isControlled = true; }

    bool isUnderControl() const { return isControlled; }
    void removeControl() { isControlled = false; }

private:
    int resources;
    Player resourceOwner;
    bool hasResourceOwner;
    int bastionDefense;
    Player controller;
    bool isControlled;
};

#endif
