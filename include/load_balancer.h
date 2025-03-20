// load_balancer.h
#ifndef LOAD_BALANCER_H
#define LOAD_BALANCER_H

#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <map>
#include <chrono>
#include <random>

// Forward declarations for optional modules
class LoadMonitor;
class ServerHealthSimulator;
class LoadPatternGenerator;

enum class BalancingAlgorithm {
    ROUND_ROBIN,
    LEAST_LOADED,
    WEIGHTED_OPTIMIZATION
};

class Server {
private:
    int id;
    int capacity;
    int currentLoad;
    double performanceMultiplier;
    bool online;
    std::string status;

public:
    Server(int id, int capacity);
    
    // Getters
    int getId() const;
    int getCapacity() const;
    int getCurrentLoad() const;
    double getPerformanceMultiplier() const;
    bool isOnline() const;
    std::string getStatus() const;
    
    // Setters
    void setCapacity(int capacity);
    void setCurrentLoad(int load);
    void setPerformanceMultiplier(double multiplier);
    void setOnline(bool online);
    void setStatus(const std::string& status);
    
    // Operations
    int getAvailableCapacity() const;
    double getEffectiveCapacity() const;
    double getLoadPercentage() const;
};

class LoadBalancer {
private:
    std::vector<std::shared_ptr<Server>> servers;
    BalancingAlgorithm currentAlgorithm;
    int nextServerId;
    int randomLoadAmount;
    std::mt19937 rng;
    
    // Optional components
    std::shared_ptr<LoadMonitor> monitor;
    std::shared_ptr<ServerHealthSimulator> healthSimulator;
    std::shared_ptr<LoadPatternGenerator> loadGenerator;
    
    // Algorithm implementations
    void distributeLoadRoundRobin(int loadAmount);
    void distributeLoadLeastLoaded(int loadAmount);
    void distributeLoadWeightedOptimization(int loadAmount);
    
    // Internal methods
    void rebalanceLoads();
    double calculateLoadVariance() const;
    int getTotalLoad() const;
    int getTotalCapacity() const;
    
    // Timing
    std::chrono::time_point<std::chrono::system_clock> lastOperationTime;
    double measureOperationTime();
    
public:
    LoadBalancer();
    ~LoadBalancer();
    
    // Server management
    void addServer(int capacity = 100);
    bool removeServer(int serverId);
    std::shared_ptr<Server> getServer(int serverId);
    const std::vector<std::shared_ptr<Server>>& getServers() const;
    
    // Load operations
    void addRandomLoad();
    void addLoadToServer(int serverId, int loadAmount);
    void addSystemLoad(int loadAmount);
    
    // Algorithm selection
    void setBalancingAlgorithm(BalancingAlgorithm algorithm);
    BalancingAlgorithm getCurrentAlgorithm() const;
    std::string getAlgorithmName() const;
    
    // Configuration
    void setRandomLoadAmount(int amount);
    int getRandomLoadAmount() const;
    
    // Visualization
    std::string visualizeLoads() const;
    std::string getSystemStatus() const;
    
    // Optional modules integration
    void attachMonitor(std::shared_ptr<LoadMonitor> monitor);
    void attachHealthSimulator(std::shared_ptr<ServerHealthSimulator> healthSimulator);
    void attachLoadGenerator(std::shared_ptr<LoadPatternGenerator> loadGenerator);
    
    // Interactive command processing
    bool processCommand(char command);
    void displayHelp() const;
    
    // Demo scenarios
    void runScalabilityDemo();

};

#endif // LOAD_BALANCER_H