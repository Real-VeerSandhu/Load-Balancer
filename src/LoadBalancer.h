#ifndef LOADBALANCER_H
#define LOADBALANCER_H

#include "Server.h"
#include <vector>
#include <string>
#include <mutex>
#include <memory>

enum class LoadBalanceStrategy {
    WEIGHTED_ROUND_ROBIN,
    LEAST_LOADED,
    ROUND_ROBIN,
    RANDOM
};

// Server snapshot structure for thread-safe monitoring
struct ServerSnapshot {
    int id;
    int power;
    int currentLoad;
    double loadPercentage;
};

class LoadBalancer {
private:
    mutable std::mutex serversMutex; // Mutex for thread-safe access (put first for alignment)
    std::vector<Server> servers;
    int nextServerIndex; // For round-robin
    LoadBalanceStrategy strategy;
    int serverIdCounter;

    // Distribution methods
    void distributeWeightedRoundRobin(int requestCount);
    void distributeLeastLoaded(int requestCount);
    void distributeRoundRobin(int requestCount);
    void distributeRandom(int requestCount);
    
    // Helper methods
    int calculateTotalPower() const;
    Server* findServerById(int id);

public:
    LoadBalancer(LoadBalanceStrategy strat = LoadBalanceStrategy::WEIGHTED_ROUND_ROBIN);
    
    // Server management
    int addServer(int power);
    bool removeServer(int id);
    
    // Request distribution
    void distributeRequests(int requestCount);
    
    // Display
    void showServerStates() const;
    
    // Strategy management
    void setStrategy(LoadBalanceStrategy strat);
    LoadBalanceStrategy getStrategy() const;
    
    // Utility
    int getServerCount() const;
    void resetAllLoads();
    
    // Thread-safe methods for Monitor
    std::vector<ServerSnapshot> getServerSnapshots() const;
    void applyRandomFluctuation(int amount);
};

#endif // LOADBALANCER_H

