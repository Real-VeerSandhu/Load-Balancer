#include "LoadBalancer.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <random>
#include <numeric>

LoadBalancer::LoadBalancer(LoadBalanceStrategy strat) 
    : nextServerIndex(0), strategy(strat), serverIdCounter(1) {
}

int LoadBalancer::addServer(int power) {
    std::lock_guard<std::mutex> lock(serversMutex);
    int newId = serverIdCounter++;
    servers.emplace_back(newId, power);
    return newId;
}

bool LoadBalancer::removeServer(int id) {
    std::lock_guard<std::mutex> lock(serversMutex);
    auto it = std::remove_if(servers.begin(), servers.end(),
        [id](const Server& s) { return s.getId() == id; });
    
    if (it != servers.end()) {
        servers.erase(it, servers.end());
        return true;
    }
    return false;
}

void LoadBalancer::distributeRequests(int requestCount) {
    std::lock_guard<std::mutex> lock(serversMutex);
    if (servers.empty()) {
        std::cout << "No servers available to handle requests!" << std::endl;
        return;
    }

    switch (strategy) {
        case LoadBalanceStrategy::WEIGHTED_ROUND_ROBIN:
            distributeWeightedRoundRobin(requestCount);
            break;
        case LoadBalanceStrategy::LEAST_LOADED:
            distributeLeastLoaded(requestCount);
            break;
        case LoadBalanceStrategy::ROUND_ROBIN:
            distributeRoundRobin(requestCount);
            break;
        case LoadBalanceStrategy::RANDOM:
            distributeRandom(requestCount);
            break;
    }
}

void LoadBalancer::distributeWeightedRoundRobin(int requestCount) {
    int totalPower = calculateTotalPower();
    if (totalPower == 0) {
        std::cout << "All servers have zero power!" << std::endl;
        return;
    }

    // Distribute requests proportionally to server power
    int remainingRequests = requestCount;
    
    for (size_t i = 0; i < servers.size() && remainingRequests > 0; ++i) {
        double proportion = static_cast<double>(servers[i].getPower()) / totalPower;
        int requestsForServer = static_cast<int>(proportion * requestCount);
        
        if (requestsForServer > remainingRequests) {
            requestsForServer = remainingRequests;
        }
        
        servers[i].addLoad(requestsForServer);
        remainingRequests -= requestsForServer;
    }
    
    // Distribute any remaining requests to servers with highest power
    if (remainingRequests > 0) {
        std::vector<size_t> indices(servers.size());
        std::iota(indices.begin(), indices.end(), 0);
        std::sort(indices.begin(), indices.end(),
            [this](size_t a, size_t b) {
                return servers[a].getPower() > servers[b].getPower();
            });
        
        for (size_t idx : indices) {
            if (remainingRequests <= 0) break;
            servers[idx].addLoad(1);
            remainingRequests--;
        }
    }
}

void LoadBalancer::distributeLeastLoaded(int requestCount) {
    for (int i = 0; i < requestCount; ++i) {
        if (servers.empty()) break;
        
        auto minIt = std::min_element(servers.begin(), servers.end(),
            [](const Server& a, const Server& b) {
                return a.getCurrentLoad() < b.getCurrentLoad();
            });
        
        minIt->addLoad(1);
    }
}

void LoadBalancer::distributeRoundRobin(int requestCount) {
    for (int i = 0; i < requestCount; ++i) {
        if (servers.empty()) break;
        
        servers[nextServerIndex].addLoad(1);
        nextServerIndex = (nextServerIndex + 1) % servers.size();
    }
}

void LoadBalancer::distributeRandom(int requestCount) {
    if (servers.empty()) return;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, servers.size() - 1);
    
    for (int i = 0; i < requestCount; ++i) {
        int randomIndex = dis(gen);
        servers[randomIndex].addLoad(1);
    }
}

void LoadBalancer::showServerStates() const {
    if (servers.empty()) {
        std::cout << "No servers configured." << std::endl;
        return;
    }

    std::cout << "\n" << std::string(60, '-') << std::endl;
    std::cout << std::left << std::setw(8) << "ID" 
              << std::setw(12) << "Power" 
              << std::setw(12) << "Load" 
              << std::setw(15) << "Load %" 
              << std::endl;
    std::cout << std::string(60, '-') << std::endl;
    
    for (const auto& server : servers) {
        std::cout << std::left 
                  << std::setw(8) << server.getId()
                  << std::setw(12) << server.getPower()
                  << std::setw(12) << server.getCurrentLoad()
                  << std::fixed << std::setprecision(2)
                  << std::setw(15) << server.getLoadPercentage() << "%"
                  << std::endl;
    }
    std::cout << std::string(60, '-') << "\n" << std::endl;
}

void LoadBalancer::setStrategy(LoadBalanceStrategy strat) {
    strategy = strat;
}

LoadBalanceStrategy LoadBalancer::getStrategy() const {
    return strategy;
}

int LoadBalancer::getServerCount() const {
    std::lock_guard<std::mutex> lock(serversMutex);
    return servers.size();
}

void LoadBalancer::resetAllLoads() {
    std::lock_guard<std::mutex> lock(serversMutex);
    for (auto& server : servers) {
        server.resetLoad();
    }
}

std::vector<ServerSnapshot> LoadBalancer::getServerSnapshots() const {
    std::lock_guard<std::mutex> lock(serversMutex);
    std::vector<ServerSnapshot> snapshots;
    snapshots.reserve(servers.size());
    
    for (const auto& server : servers) {
        ServerSnapshot snap;
        snap.id = server.getId();
        snap.power = server.getPower();
        snap.currentLoad = server.getCurrentLoad();
        snap.loadPercentage = server.getLoadPercentage();
        snapshots.push_back(snap);
    }
    
    return snapshots;
}

void LoadBalancer::applyRandomFluctuation(int amount) {
    std::lock_guard<std::mutex> lock(serversMutex);
    if (servers.empty()) return;
    
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, servers.size() - 1);
    
    int randomIndex = dis(gen);
    int newLoad = servers[randomIndex].getCurrentLoad() + amount;
    if (newLoad < 0) newLoad = 0;
    
    // Reset and set new load
    servers[randomIndex].resetLoad();
    if (newLoad > 0) {
        servers[randomIndex].addLoad(newLoad);
    }
}

int LoadBalancer::calculateTotalPower() const {
    int total = 0;
    for (const auto& server : servers) {
        total += server.getPower();
    }
    return total;
}

Server* LoadBalancer::findServerById(int id) {
    for (auto& server : servers) {
        if (server.getId() == id) {
            return &server;
        }
    }
    return nullptr;
}

