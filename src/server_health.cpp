// server_health.cc
#include "server_health.h"
#include <algorithm>
#include <iostream>

ServerHealthSimulator::ServerHealthSimulator() 
    : rng(std::random_device{}()) {
    
    // Initialize state labels
    stateLabels[ServerState::HEALTHY] = "HEALTHY";
    stateLabels[ServerState::DEGRADED] = "DEGRADED";
    stateLabels[ServerState::CRITICAL] = "CRITICAL";
    stateLabels[ServerState::OFFLINE] = "OFFLINE";
}

void ServerHealthSimulator::addServer(int serverId) {
    // Check if server already exists
    auto it = std::find_if(servers.begin(), servers.end(),
                           [serverId](const ServerHealth& sh) { return sh.serverId == serverId; });
    
    if (it != servers.end()) {
        std::cerr << "Server ID " << serverId << " already exists in health simulator" << std::endl;
        return;
    }
    
    // Add new server with default healthy state
    ServerHealth newServer;
    newServer.serverId = serverId;
    newServer.state = ServerState::HEALTHY;
    newServer.healthScore = 1.0;
    newServer.failureProbability = 0.01;  // 1% chance of failure per update
    newServer.recoveryProbability = 0.2;  // 20% chance of recovery per update when degraded
    newServer.lastStateChange = std::chrono::system_clock::now();
    newServer.performanceMultiplier = 1.0;
    
    servers.push_back(newServer);
}

void ServerHealthSimulator::removeServer(int serverId) {
    servers.erase(
        std::remove_if(servers.begin(), servers.end(),
                      [serverId](const ServerHealth& sh) { return sh.serverId == serverId; }),
        servers.end()
    );
}

void ServerHealthSimulator::updateServerStates() {
    std::uniform_real_distribution<> dist(0.0, 1.0);
    
    for (auto& server : servers) {
        auto now = std::chrono::system_clock::now();
        double elapsedSeconds = std::chrono::duration<double>(now - server.lastStateChange).count();
        
        // Skip updating servers that changed state less than 5 seconds ago
        if (elapsedSeconds < 5.0) {
            continue;
        }
        
        double randomValue = dist(rng);
        
        switch (server.state) {
            case ServerState::HEALTHY:
                // Chance of degradation
                if (randomValue < server.failureProbability) {
                    server.state = ServerState::DEGRADED;
                    server.healthScore = 0.7;  // Degraded health score
                    server.performanceMultiplier = 0.7;  // 70% performance
                    server.lastStateChange = now;
                    
                    if (stateChangeCallback) {
                        stateChangeCallback(server.serverId, server.state);
                    }
                    
                    if (performanceUpdateCallback) {
                        performanceUpdateCallback(server.serverId, server.performanceMultiplier);
                    }
                }
                break;
                
            case ServerState::DEGRADED:
                // Chance of recovery or further degradation
                if (randomValue < server.recoveryProbability) {
                    // Recover
                    server.state = ServerState::HEALTHY;
                    server.healthScore = 1.0;
                    server.performanceMultiplier = 1.0;
                    server.lastStateChange = now;
                    
                    if (stateChangeCallback) {
                        stateChangeCallback(server.serverId, server.state);
                    }
                    
                    if (performanceUpdateCallback) {
                        performanceUpdateCallback(server.serverId, server.performanceMultiplier);
                    }
                } 
                else if (randomValue > (1.0 - server.failureProbability * 2)) {
                    // Further degradation
                    server.state = ServerState::CRITICAL;
                    server.healthScore = 0.3;
                    server.performanceMultiplier = 0.4;  // 40% performance
                    server.lastStateChange = now;
                    
                    if (stateChangeCallback) {
                        stateChangeCallback(server.serverId, server.state);
                    }
                    
                    if (performanceUpdateCallback) {
                        performanceUpdateCallback(server.serverId, server.performanceMultiplier);
                    }
                }
                break;
                
            case ServerState::CRITICAL:
                // Chance of recovery or going offline
                if (randomValue < server.recoveryProbability / 2) {
                    // Partial recovery
                    server.state = ServerState::DEGRADED;
                    server.healthScore = 0.6;
                    server.performanceMultiplier = 0.6;
                    server.lastStateChange = now;
                    
                    if (stateChangeCallback) {
                        stateChangeCallback(server.serverId, server.state);
                    }
                    
                    if (performanceUpdateCallback) {
                        performanceUpdateCallback(server.serverId, server.performanceMultiplier);
                    }
                } 
                else if (randomValue > (1.0 - server.failureProbability * 3)) {
                    // Go offline
                    server.state = ServerState::OFFLINE;
                    server.healthScore = 0.0;
                    server.performanceMultiplier = 0.0;  // No performance
                    server.lastStateChange = now;
                    
                    if (stateChangeCallback) {
                        stateChangeCallback(server.serverId, server.state);
                    }
                    
                    if (performanceUpdateCallback) {
                        performanceUpdateCallback(server.serverId, server.performanceMultiplier);
                    }
                }
                break;
                
            case ServerState::OFFLINE:
                // Chance of recovery
                if (randomValue < server.recoveryProbability / 3) {
                    server.state = ServerState::CRITICAL;
                    server.healthScore = 0.2;
                    server.performanceMultiplier = 0.3;
                    server.lastStateChange = now;
                    
                    if (stateChangeCallback) {
                        stateChangeCallback(server.serverId, server.state);
                    }
                    
                    if (performanceUpdateCallback) {
                        performanceUpdateCallback(server.serverId, server.performanceMultiplier);
                    }
                }
                break;
        }
    }
}

ServerState ServerHealthSimulator::getServerState(int serverId) const {
    auto it = std::find_if(servers.begin(), servers.end(),
                           [serverId](const ServerHealth& sh) { return sh.serverId == serverId; });
    
    if (it != servers.end()) {
        return it->state;
    }
    
    // Default to HEALTHY if server not found
    return ServerState::HEALTHY;
}

double ServerHealthSimulator::getServerHealthScore(int serverId) const {
    auto it = std::find_if(servers.begin(), servers.end(),
                           [serverId](const ServerHealth& sh) { return sh.serverId == serverId; });
    
    if (it != servers.end()) {
        return it->healthScore;
    }
    
    // Default to 1.0 if server not found
    return 1.0;
}

double ServerHealthSimulator::getServerPerformanceMultiplier(int serverId) const {
    auto it = std::find_if(servers.begin(), servers.end(),
                           [serverId](const ServerHealth& sh) { return sh.serverId == serverId; });
    
    if (it != servers.end()) {
        return it->performanceMultiplier;
    }
    
    // Default to 1.0 if server not found
    return 1.0;
}

std::string ServerHealthSimulator::getServerStateLabel(int serverId) const {
    ServerState state = getServerState(serverId);
    return stateLabels.at(state);
}

void ServerHealthSimulator::setServerState(int serverId, ServerState state) {
    auto it = std::find_if(servers.begin(), servers.end(),
                           [serverId](const ServerHealth& sh) { return sh.serverId == serverId; });
    
    if (it != servers.end()) {
        it->state = state;
        it->lastStateChange = std::chrono::system_clock::now();
        
        // Update health score and performance multiplier based on state
        switch (state) {
            case ServerState::HEALTHY:
                it->healthScore = 1.0;
                it->performanceMultiplier = 1.0;
                break;
                
            case ServerState::DEGRADED:
                it->healthScore = 0.7;
                it->performanceMultiplier = 0.7;
                break;
                
            case ServerState::CRITICAL:
                it->healthScore = 0.3;
                it->performanceMultiplier = 0.4;
                break;
                
            case ServerState::OFFLINE:
                it->healthScore = 0.0;
                it->performanceMultiplier = 0.0;
                break;
        }
        
        if (stateChangeCallback) {
            stateChangeCallback(serverId, state);
        }
        
        if (performanceUpdateCallback) {
            performanceUpdateCallback(serverId, it->performanceMultiplier);
        }
    }
}

void ServerHealthSimulator::degradeServerPerformance(int serverId, double degradationFactor) {
    auto it = std::find_if(servers.begin(), servers.end(),
                           [serverId](const ServerHealth& sh) { return sh.serverId == serverId; });
    
    if (it != servers.end() && it->state != ServerState::OFFLINE) {
        // Ensure degradationFactor is valid (between 0 and 1)
        degradationFactor = std::max(0.0, std::min(1.0, degradationFactor));
        
        // Apply degradation
        it->performanceMultiplier *= degradationFactor;
        
        // Update state based on new performance level
        if (it->performanceMultiplier < 0.1) {
            it->state = ServerState::OFFLINE;
            it->healthScore = 0.0;
            it->performanceMultiplier = 0.0;
        } 
        else if (it->performanceMultiplier < 0.5) {
            it->state = ServerState::CRITICAL;
            it->healthScore = 0.3;
        } 
        else if (it->performanceMultiplier < 0.9) {
            it->state = ServerState::DEGRADED;
            it->healthScore = 0.7;
        }
        
        it->lastStateChange = std::chrono::system_clock::now();
        
        if (stateChangeCallback) {
            stateChangeCallback(serverId, it->state);
        }
        
        if (performanceUpdateCallback) {
            performanceUpdateCallback(serverId, it->performanceMultiplier);
        }
    }
}

void ServerHealthSimulator::recoverServer(int serverId) {
    auto it = std::find_if(servers.begin(), servers.end(),
                           [serverId](const ServerHealth& sh) { return sh.serverId == serverId; });
    
    if (it != servers.end()) {
        it->state = ServerState::HEALTHY;
        it->healthScore = 1.0;
        it->performanceMultiplier = 1.0;
        it->lastStateChange = std::chrono::system_clock::now();
        
        if (stateChangeCallback) {
            stateChangeCallback(serverId, ServerState::HEALTHY);
        }
        
        if (performanceUpdateCallback) {
            performanceUpdateCallback(serverId, 1.0);
        }
    }
}

void ServerHealthSimulator::setStateChangeCallback(std::function<void(int, ServerState)> callback) {
    stateChangeCallback = callback;
}

void ServerHealthSimulator::setPerformanceUpdateCallback(std::function<void(int, double)> callback) {
    performanceUpdateCallback = callback;
}

void ServerHealthSimulator::simulateRandomFailure() {
    if (servers.empty()) return;
    
    std::uniform_int_distribution<> distInt(0, servers.size() - 1);
    int index = distInt(rng);
    
    std::uniform_real_distribution<> distSeverity(0.0, 1.0);
    double severity = distSeverity(rng);
    
    if (severity < 0.2) {
        // Minor degradation
        setServerState(servers[index].serverId, ServerState::DEGRADED);
    } 
    else if (severity < 0.7) {
        // Critical condition
        setServerState(servers[index].serverId, ServerState::CRITICAL);
    } 
    else {
        // Complete failure
        setServerState(servers[index].serverId, ServerState::OFFLINE);
    }
}

void ServerHealthSimulator::simulateNetworkPartition(const std::vector<int>& affectedServers) {
    for (int serverId : affectedServers) {
        // Network partition completely cuts off servers
        setServerState(serverId, ServerState::OFFLINE);
    }
}

void ServerHealthSimulator::simulateHighLoad(int serverId) {
    auto it = std::find_if(servers.begin(), servers.end(),
                          [serverId](const ServerHealth& sh) { return sh.serverId == serverId; });
    
    if (it != servers.end() && it->state != ServerState::OFFLINE) {
        // High load causes performance degradation
        std::uniform_real_distribution<> dist(0.5, 0.8);
        double degradationFactor = dist(rng);
        
        degradeServerPerformance(serverId, degradationFactor);
    }
}

std::string ServerHealthSimulator::stateToString(ServerState state) {
    switch (state) {
        case ServerState::HEALTHY:  return "HEALTHY";
        case ServerState::DEGRADED: return "DEGRADED";
        case ServerState::CRITICAL: return "CRITICAL";
        case ServerState::OFFLINE:  return "OFFLINE";
        default: return "UNKNOWN";
    }
}