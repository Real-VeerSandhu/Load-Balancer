#include "Server.h"
#include <algorithm>

Server::Server(int serverId, int serverPower) 
    : id(serverId), power(serverPower), currentLoad(0) {
}

void Server::addLoad(int amount) {
    currentLoad += amount;
}

bool Server::canHandle(int amount) const {
    // A server can handle a request if adding it won't exceed its power
    // In a real scenario, you might want to check against capacity
    return (currentLoad + amount) <= (power * 10); // Allow some headroom
}

void Server::resetLoad() {
    currentLoad = 0;
}

int Server::getId() const {
    return id;
}

int Server::getPower() const {
    return power;
}

int Server::getCurrentLoad() const {
    return currentLoad;
}

double Server::getLoadPercentage() const {
    if (power == 0) return 0.0;
    return (static_cast<double>(currentLoad) / power) * 100.0;
}

