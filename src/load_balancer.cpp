// load_balancer.cpp
#include "include/load_balancer.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <thread>
#include <chrono>
#include <limits>

// Uncomment these when you want to use the optional modules
// #include "monitoring.h"
// #include "server_health.h"
// #include "load_pattern.h"

// Server implementation
Server::Server(int id, int capacity) 
    : id(id), capacity(capacity), currentLoad(0), performanceMultiplier(1.0), online(true), status("HEALTHY") {
}

int Server::getId() const {
    return id;
}

int Server::getCapacity() const {
    return capacity;
}

int Server::getCurrentLoad() const {
    return currentLoad;
}

double Server::getPerformanceMultiplier() const {
    return performanceMultiplier;
}

bool Server::isOnline() const {
    return online;
}

std::string Server::getStatus() const {
    return status;
}

void Server::setCapacity(int capacity) {
    this->capacity = capacity;
}

void Server::setCurrentLoad(int load) {
    this->currentLoad = load;
    if (this->currentLoad < 0) {
        this->currentLoad = 0;
    }
}

void Server::setPerformanceMultiplier(double multiplier) {
    this->performanceMultiplier = multiplier;
    if (this->performanceMultiplier < 0.0) {
        this->performanceMultiplier = 0.0;
    } else if (this->performanceMultiplier > 1.0) {
        this->performanceMultiplier = 1.0;
    }
}

void Server::setOnline(bool online) {
    this->online = online;
}

void Server::setStatus(const std::string& status) {
    this->status = status;
}

int Server::getAvailableCapacity() const {
    if (!online) return 0;
    return capacity - currentLoad;
}

double Server::getEffectiveCapacity() const {
    if (!online) return 0.0;
    return capacity * performanceMultiplier;
}

double Server::getLoadPercentage() const {
    if (capacity == 0) return 0.0;
    return (static_cast<double>(currentLoad) / capacity) * 100.0;
}

// LoadBalancer implementation
LoadBalancer::LoadBalancer() 
    : currentAlgorithm(BalancingAlgorithm::ROUND_ROBIN), 
      nextServerId(1),
      randomLoadAmount(10),
      rng(std::random_device{}()) {
    
    // Initialize with a few servers
    for (int i = 0; i < 3; ++i) {
        addServer();
    }
    
    lastOperationTime = std::chrono::system_clock::now();
}

LoadBalancer::~LoadBalancer() {
    // Clean up if needed
}

void LoadBalancer::addServer(int capacity) {
    auto server = std::make_shared<Server>(nextServerId++, capacity);
    servers.push_back(server);
    
    // Notify health simulator if attached
    if (healthSimulator) {
        // This would be implemented if you include the health simulator header
        // healthSimulator->addServer(server->getId());
    }
    
    std::cout << "Server #" << server->getId() << " added with capacity " << capacity << std::endl;
}

bool LoadBalancer::removeServer(int serverId) {
    auto it = std::find_if(servers.begin(), servers.end(),
                          [serverId](const std::shared_ptr<Server>& s) { 
                              return s->getId() == serverId; 
                          });
    
    if (it == servers.end()) {
        std::cout << "Server #" << serverId << " not found" << std::endl;
        return false;
    }
    
    // Redistribute load from the server being removed
    int loadToRedistribute = (*it)->getCurrentLoad();
    
    // Remove server
    servers.erase(it);
    
    // Notify health simulator if attached
    if (healthSimulator) {
        // This would be implemented if you include the health simulator header
        // healthSimulator->removeServer(serverId);
    }
    
    std::cout << "Server #" << serverId << " removed" << std::endl;
    
    // Redistribute load if there are servers remaining
    if (!servers.empty() && loadToRedistribute > 0) {
        std::cout << "Redistributing " << loadToRedistribute << " load units..." << std::endl;
        addSystemLoad(loadToRedistribute);
    }
    
    return true;
}

std::shared_ptr<Server> LoadBalancer::getServer(int serverId) {
    auto it = std::find_if(servers.begin(), servers.end(),
                          [serverId](const std::shared_ptr<Server>& s) { 
                              return s->getId() == serverId; 
                          });
    
    if (it != servers.end()) {
        return *it;
    }
    
    return nullptr;
}

const std::vector<std::shared_ptr<Server>>& LoadBalancer::getServers() const {
    return servers;
}

void LoadBalancer::distributeLoadRoundRobin(int loadAmount) {
    if (servers.empty()) {
        std::cout << "No servers available to distribute load" << std::endl;
        return;
    }
    
    // Find the first online server
    size_t startIdx = 0;
    while (startIdx < servers.size() && !servers[startIdx]->isOnline()) {
        startIdx++;
    }
    
    if (startIdx >= servers.size()) {
        std::cout << "No online servers available" << std::endl;
        return;
    }
    
    // Start with a static distribution
    int baseLoadPerServer = loadAmount / servers.size();
    int remainingLoad = loadAmount % servers.size();
    
    // Distribute base load to all servers
    for (size_t i = 0; i < servers.size(); i++) {
        size_t idx = (startIdx + i) % servers.size();
        
        if (servers[idx]->isOnline()) {
            int serverLoad = baseLoadPerServer;
            
            // Distribute remaining load units one by one
            if (remainingLoad > 0) {
                serverLoad++;
                remainingLoad--;
            }
            
            servers[idx]->setCurrentLoad(servers[idx]->getCurrentLoad() + serverLoad);
        }
    }
}

void LoadBalancer::distributeLoadLeastLoaded(int loadAmount) {
    if (servers.empty()) {
        std::cout << "No servers available to distribute load" << std::endl;
        return;
    }
    
    // Track remaining load to distribute
    int remainingLoad = loadAmount;
    
    // Keep distributing while there's load and available capacity
    while (remainingLoad > 0) {
        // Find server with the most available capacity
        std::shared_ptr<Server> bestServer = nullptr;
        int bestAvailableCapacity = -1;
        
        for (auto& server : servers) {
            if (!server->isOnline()) continue;
            
            int availableCapacity = server->getAvailableCapacity();
            if (availableCapacity > bestAvailableCapacity) {
                bestAvailableCapacity = availableCapacity;
                bestServer = server;
            }
        }
        
        // No more capacity available
        if (!bestServer || bestAvailableCapacity <= 0) {
            std::cout << "Warning: Insufficient capacity. " << remainingLoad 
                      << " load units could not be distributed." << std::endl;
            break;
        }
        
        // Determine how much load to add to this server
        int loadToAdd = std::min(remainingLoad, bestAvailableCapacity);
        bestServer->setCurrentLoad(bestServer->getCurrentLoad() + loadToAdd);
        remainingLoad -= loadToAdd;
    }
}

void LoadBalancer::distributeLoadWeightedOptimization(int loadAmount) {
    if (servers.empty()) {
        std::cout << "No servers available to distribute load" << std::endl;
        return;
    }
    
    // Calculate total effective capacity
    double totalEffectiveCapacity = 0.0;
    for (auto& server : servers) {
        if (server->isOnline()) {
            totalEffectiveCapacity += server->getEffectiveCapacity();
        }
    }
    
    if (totalEffectiveCapacity <= 0.0) {
        std::cout << "No effective capacity available" << std::endl;
        return;
    }
    
    // Calculate ideal load distribution based on capacity ratio
    std::vector<int> idealLoads(servers.size());
    int distributedLoad = 0;
    
    for (size_t i = 0; i < servers.size(); i++) {
        if (!servers[i]->isOnline()) {
            idealLoads[i] = 0;
            continue;
        }
        
        // Calculate proportional load
        double ratio = servers[i]->getEffectiveCapacity() / totalEffectiveCapacity;
        int serverIdealLoad = static_cast<int>(ratio * loadAmount);
        
        // Ensure we don't exceed capacity
        int availableCapacity = servers[i]->getAvailableCapacity();
        if (serverIdealLoad > availableCapacity) {
            serverIdealLoad = availableCapacity;
        }
        
        idealLoads[i] = serverIdealLoad;
        distributedLoad += serverIdealLoad;
    }
    
    // Distribute any remaining load to servers that still have capacity
    int remainingLoad = loadAmount - distributedLoad;
    while (remainingLoad > 0) {
        bool distributed = false;
        
        for (size_t i = 0; i < servers.size() && remainingLoad > 0; i++) {
            if (!servers[i]->isOnline()) continue;
            
            int availableCapacity = servers[i]->getAvailableCapacity() - idealLoads[i];
            if (availableCapacity > 0) {
                idealLoads[i]++;
                remainingLoad--;
                distributed = true;
            }
        }
        
        if (!distributed) break; // No more capacity
    }
    
    // Apply the calculated loads
    for (size_t i = 0; i < servers.size(); i++) {
        if (idealLoads[i] > 0) {
            servers[i]->setCurrentLoad(servers[i]->getCurrentLoad() + idealLoads[i]);
        }
    }
    
    if (remainingLoad > 0) {
        std::cout << "Warning: Insufficient capacity. " << remainingLoad 
                  << " load units could not be distributed." << std::endl;
    }
}

void LoadBalancer::rebalanceLoads() {
    // Calculate total current load
    int totalLoad = getTotalLoad();
    
    // Reset all server loads to zero
    for (auto& server : servers) {
        server->setCurrentLoad(0);
    }
    
    // Redistribute total load using current algorithm
    addSystemLoad(totalLoad);
    
    std::cout << "Load rebalanced using " << getAlgorithmName() << " algorithm" << std::endl;
}

double LoadBalancer::calculateLoadVariance() const {
    if (servers.empty()) return 0.0;
    
    // Calculate mean load percentage
    double totalPercentage = 0.0;
    int onlineCount = 0;
    
    for (auto& server : servers) {
        if (server->isOnline()) {
            totalPercentage += server->getLoadPercentage();
            onlineCount++;
        }
    }
    
    if (onlineCount == 0) return 0.0;
    
    double meanPercentage = totalPercentage / onlineCount;
    
    // Calculate variance
    double variance = 0.0;
    for (auto& server : servers) {
        if (server->isOnline()) {
            double diff = server->getLoadPercentage() - meanPercentage;
            variance += diff * diff;
        }
    }
    
    return variance / onlineCount;
}

int LoadBalancer::getTotalLoad() const {
    int total = 0;
    for (auto& server : servers) {
        total += server->getCurrentLoad();
    }
    return total;
}

int LoadBalancer::getTotalCapacity() const {
    int total = 0;
    for (auto& server : servers) {
        if (server->isOnline()) {
            total += server->getCapacity();
        }
    }
    return total;
}

double LoadBalancer::measureOperationTime() {
    auto now = std::chrono::system_clock::now();
    double elapsed = std::chrono::duration<double, std::milli>(now - lastOperationTime).count();
    lastOperationTime = now;
    return elapsed;
}

void LoadBalancer::addRandomLoad() {
    addSystemLoad(randomLoadAmount);
}

void LoadBalancer::addLoadToServer(int serverId, int loadAmount) {
    auto server = getServer(serverId);
    if (!server) {
        std::cout << "Server #" << serverId << " not found" << std::endl;
        return;
    }
    
    if (!server->isOnline()) {
        std::cout << "Server #" << serverId << " is offline" << std::endl;
        return;
    }
    
    int availableCapacity = server->getAvailableCapacity();
    if (loadAmount > availableCapacity) {
        std::cout << "Warning: Exceeding server capacity. Only " 
                  << availableCapacity << " load units added." << std::endl;
        loadAmount = availableCapacity;
    }
    
    server->setCurrentLoad(server->getCurrentLoad() + loadAmount);
    std::cout << "Added " << loadAmount << " load units to Server #" << serverId << std::endl;
    
    // Record operation time for monitoring
    double operationTime = measureOperationTime();
    
    // Update monitor if attached
    if (monitor) {
        std::vector<int> loads;
        for (auto& s : servers) {
            loads.push_back(s->getCurrentLoad());
        }
        // This would be implemented if you include the monitoring header
        // monitor->recordMetrics(loads, operationTime);
    }
}

void LoadBalancer::addSystemLoad(int loadAmount) {
    std::cout << "Adding " << loadAmount << " load units using " 
              << getAlgorithmName() << " algorithm" << std::endl;
    
    // Distribute load according to current algorithm
    switch (currentAlgorithm) {
        case BalancingAlgorithm::ROUND_ROBIN:
            distributeLoadRoundRobin(loadAmount);
            break;
            
        case BalancingAlgorithm::LEAST_LOADED:
            distributeLoadLeastLoaded(loadAmount);
            break;
            
        case BalancingAlgorithm::WEIGHTED_OPTIMIZATION:
            distributeLoadWeightedOptimization(loadAmount);
            break;
    }
    
    // Record operation time for monitoring
    double operationTime = measureOperationTime();
    
    // Update monitor if attached
    if (monitor) {
        std::vector<int> loads;
        for (auto& server : servers) {
            loads.push_back(server->getCurrentLoad());
        }
        // This would be implemented if you include the monitoring header
        // monitor->recordMetrics(loads, operationTime);
    }
    
    // Display updated system
    std::cout << visualizeLoads() << std::endl;
}

void LoadBalancer::setBalancingAlgorithm(BalancingAlgorithm algorithm) {
    currentAlgorithm = algorithm;
    std::cout << "Switched to " << getAlgorithmName() << " algorithm" << std::endl;
    
    // Update monitor if attached
    if (monitor) {
        // This would be implemented if you include the monitoring header
        // monitor->setAlgorithm(getAlgorithmName());
    }
}

BalancingAlgorithm LoadBalancer::getCurrentAlgorithm() const {
    return currentAlgorithm;
}

std::string LoadBalancer::getAlgorithmName() const {
    switch (currentAlgorithm) {
        case BalancingAlgorithm::ROUND_ROBIN:
            return "Round Robin";
        case BalancingAlgorithm::LEAST_LOADED:
            return "Least Loaded";
        case BalancingAlgorithm::WEIGHTED_OPTIMIZATION:
            return "Weighted Optimization";
        default:
            return "Unknown";
    }
}

void LoadBalancer::setRandomLoadAmount(int amount) {
    randomLoadAmount = amount;
    std::cout << "Random load amount set to " << randomLoadAmount << std::endl;
}

int LoadBalancer::getRandomLoadAmount() const {
    return randomLoadAmount;
}

std::string LoadBalancer::visualizeLoads() const {
    std::stringstream ss;
    
    // Calculate the maximum capacity for scaling
    int maxCapacity = 0;
    for (auto& server : servers) {
        if (server->getCapacity() > maxCapacity) {
            maxCapacity = server->getCapacity();
        }
    }
    
    // Display header
    ss << "System Load Visualization:" << std::endl;
    
    // Display each server's load
    const int barWidth = 40; // Maximum width of the load bar
    
    for (auto& server : servers) {
        ss << "Server #" << std::setw(2) << server->getId() << " ";
        
        // Add status indicator
        if (!server->isOnline()) {
            ss << "[OFFLINE] ";
        } else {
            ss << "[" << server->getStatus() << "] ";
        }
        
        // Calculate load percentage and bar length
        double percentage = server->getLoadPercentage();
        int loadBarLength = static_cast<int>((percentage / 100.0) * barWidth);
        
        // Draw the load bar
        ss << "[";
        for (int i = 0; i < barWidth; i++) {
            if (i < loadBarLength) {
                ss << "#";
            } else {
                ss << " ";
            }
        }
        
        ss << "] " << std::fixed << std::setprecision(1) << percentage << "%" 
           << " (" << server->getCurrentLoad() << "/" << server->getCapacity() << ")" << std::endl;
    }
    
    // Display system statistics
    double variance = calculateLoadVariance();
    int totalLoad = getTotalLoad();
    int totalCapacity = getTotalCapacity();
    double systemLoadPercentage = (totalCapacity > 0) ? 
                                  (static_cast<double>(totalLoad) / totalCapacity) * 100.0 : 0.0;
    
    ss << "System Load: " << totalLoad << "/" << totalCapacity 
       << " (" << std::fixed << std::setprecision(1) << systemLoadPercentage << "%)" << std::endl;
    ss << "Load Variance: " << std::fixed << std::setprecision(2) << variance << std::endl;
    ss << "Current Algorithm: " << getAlgorithmName() << std::endl;
    
    return ss.str();
}

std::string LoadBalancer::getSystemStatus() const {
    std::stringstream ss;
    
    ss << "=== LOAD BALANCER SYSTEM STATUS ===" << std::endl;
    ss << "Active Servers: " << servers.size() << std::endl;
    ss << "Total System Capacity: " << getTotalCapacity() << std::endl;
    ss << "Current Total Load: " << getTotalLoad() << std::endl;
    ss << "Load Balancing Algorithm: " << getAlgorithmName() << std::endl;
    ss << "Random Load Amount: " << randomLoadAmount << std::endl;
    
    // Add more status information as needed
    
    return ss.str();
}

void LoadBalancer::attachMonitor(std::shared_ptr<LoadMonitor> monitorObj) {
    monitor = monitorObj;
    std::cout << "Load monitor attached" << std::endl;
    
    // Initial setup
    if (monitor) {
        // This would be implemented if you include the monitoring header
        // monitor->setAlgorithm(getAlgorithmName());
    }
}

void LoadBalancer::attachHealthSimulator(std::shared_ptr<ServerHealthSimulator> healthSimObj) {
    healthSimulator = healthSimObj;
    std::cout << "Server health simulator attached" << std::endl;
    
    // Register existing servers with the health simulator
    if (healthSimulator) {
        for (auto& server : servers) {
            // This would be implemented if you include the health simulator header
            // healthSimulator->addServer(server->getId());
        }
        
        // Set callbacks from health simulator to update server health
        // This would be implemented if you include the health simulator header
        /*
        healthSimulator->setStateChangeCallback([this](int serverId, ServerState state) {
            auto server = this->getServer(serverId);
            if (server) {
                server->setStatus(ServerHealthSimulator::stateToString(state));
                server->setOnline(state != ServerState::OFFLINE);
            }
        });
        
        healthSimulator->setPerformanceUpdateCallback([this](int serverId, double multiplier) {
            auto server = this->getServer(serverId);
            if (server) {
                server->setPerformanceMultiplier(multiplier);
            }
        });
        */
    }
}

void LoadBalancer::attachLoadGenerator(std::shared_ptr<LoadPatternGenerator> loadGenObj) {
    loadGenerator = loadGenObj;
    std::cout << "Load pattern generator attached" << std::endl;
    
    // Set callback from load generator to add load
    if (loadGenerator) {
        // This would be implemented if you include the load pattern generator header
        /*
        loadGenerator->setLoadGeneratedCallback([this](int loadAmount) {
            this->addSystemLoad(loadAmount);
        });
        */
    }
}

bool LoadBalancer::processCommand(char command) {
    switch (command) {
        case 'a':
            addRandomLoad();
            return true;
            
        case 's':
            addServer();
            return true;
            
        case 'd': {
            // Remove the server with the highest ID
            if (!servers.empty()) {
                int highestId = 0;
                for (auto& server : servers) {
                    if (server->getId() > highestId) {
                        highestId = server->getId();
                    }
                }
                removeServer(highestId);
            } else {
                std::cout << "No servers to remove" << std::endl;
            }
            return true;
        }
            
        case 'r':
            rebalanceLoads();
            return true;
            
        case 'm': {
            // Cycle through algorithms
            int algo = static_cast<int>(currentAlgorithm);
            algo = (algo + 1) % 3;  // Assuming 3 algorithms
            setBalancingAlgorithm(static_cast<BalancingAlgorithm>(algo));
            return true;
        }
            
        case '+':
            setRandomLoadAmount(randomLoadAmount + 5);
            return true;
            
        case '-':
            if (randomLoadAmount > 5) {
                setRandomLoadAmount(randomLoadAmount - 5);
            }
            return true;
            
        case 'h':
            displayHelp();
            return true;
            
        case 'q':
            std::cout << "Exiting simulation..." << std::endl;
            return false;  // Signal to quit
            
        default:
            // Check if it's a digit (1-9) to add load to a specific server
            if (command >= '1' && command <= '9') {
                int serverId = command - '0';
                addLoadToServer(serverId, randomLoadAmount);
                return true;
            }
            
            std::cout << "Unknown command. Type 'h' for help." << std::endl;
            return true;
    }
}

void LoadBalancer::displayHelp() const {
    std::cout << "=== LOAD BALANCER SIMULATION COMMANDS ===" << std::endl;
    std::cout << "a: Add random load to the system" << std::endl;
    std::cout << "s: Add a new server" << std::endl;
    std::cout << "d: Remove a server" << std::endl;
    std::cout << "r: Rebalance all loads using current algorithm" << std::endl;
    std::cout << "m: Switch between optimization algorithms" << std::endl;
    std::cout << "1-9: Add load to a specific server (by ID)" << std::endl;
    std::cout << "+/-: Increase/decrease the random load amount" << std::endl;
    std::cout << "h: Display this help message" << std::endl;
    std::cout << "q: Quit the simulation" << std::endl;
    std::cout << "=========================================" << std::endl;
}

void LoadBalancer::runScalabilityDemo() {
    std::cout << "=== RUNNING SCALABILITY DEMO ===" << std::endl;
    std::cout << "Starting with 3 servers and gradually scaling up to 8..." << std::endl;
    
    // Ensure we have 3 servers to start
    while (servers.size() > 3) {
        servers.pop_back();
    }
    
    while (servers.size() < 3) {
        addServer();
    }
    
    // Reset all loads
    for (auto& server : servers) {
        server->setCurrentLoad(0);
    }
    
    // Set to Round Robin algorithm
    setBalancingAlgorithm(BalancingAlgorithm::ROUND_ROBIN);
    
    // Display initial state
    std::cout << "Initial state:" << std::endl;
    std::cout << visualizeLoads() << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // Add some initial load
    for (int i = 0; i < 5; i++) {
        addSystemLoad(20);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    
    // Start adding servers and more load
    for (int i = 0; i < 5; i++) {
        std::cout << "Adding new server and more load..." << std::endl;
        addServer();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        addSystemLoad(30);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        // Switch algorithms periodically
        if (i % 2 == 0) {
            setBalancingAlgorithm(BalancingAlgorithm::LEAST_LOADED);
        } else {
            setBalancingAlgorithm(BalancingAlgorithm::WEIGHTED_OPTIMIZATION);
        }
        
        // Rebalance
        rebalanceLoads();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    std::cout << "Final state:" << std::endl;
    std::cout << visualizeLoads() << std::endl;
    std::cout << "=== SCALABILITY DEMO COMPLETED ===" << std::endl;
}