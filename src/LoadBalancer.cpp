#include "LoadBalancer.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <random>
#include <numeric>

LoadBalancer::LoadBalancer(LoadBalanceStrategy strat) 
    : nextServerIndex(0), strategy(strat), serverIdCounter(1) {
    // serversMutex and servers are default-constructed automatically
    // Note: mutex initialization may trigger AddressSanitizer false positives on some platforms
    // This is a known issue and can be safely ignored
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

void LoadBalancer::applyNaturalFluctuations(double fluctuationRate, int maxFluctuationAmount) {
    std::lock_guard<std::mutex> lock(serversMutex);
    if (servers.empty()) return;
    
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<> probDist(0.0, 1.0);
    static std::uniform_int_distribution<> amountDist(-maxFluctuationAmount, maxFluctuationAmount);
    
    // Apply small random fluctuations to all servers (like real-world traffic variations)
    for (auto& server : servers) {
        int currentLoad = server.getCurrentLoad();
        int totalChange = 0;
        
        // Apply random fluctuation - every server gets some variation
        // Use fluctuationRate to determine how often significant changes happen
        if (probDist(gen) < fluctuationRate) {
            // Significant fluctuation
            int fluctuation = amountDist(gen);
            totalChange += fluctuation;
        } else {
            // Small random variation (always apply some tiny change)
            std::uniform_int_distribution<> smallDist(-1, 1);
            totalChange += smallDist(gen);
        }
        
        // Natural decay - servers process requests over time
        // Apply decay more frequently for servers with load
        if (currentLoad > 0) {
            // Higher chance of decay for loaded servers
            double decayChance = std::min(0.3, currentLoad / 100.0);
            if (probDist(gen) < decayChance) {
                // Decay 1-3% of current load
                int decayPercent = 1 + (static_cast<int>(probDist(gen) * 100) % 3); // 1-3%
                int decay = static_cast<int>(currentLoad * decayPercent / 100.0);
                if (decay > 0) {
                    totalChange -= decay;
                }
            }
        }
        
        // Apply the total change
        int newLoad = currentLoad + totalChange;
        if (newLoad < 0) {
            newLoad = 0;
        }
        
        // Update the server load
        server.resetLoad();
        if (newLoad > 0) {
            server.addLoad(newLoad);
        }
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

