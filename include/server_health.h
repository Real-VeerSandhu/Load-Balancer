// server_health.h
#ifndef SERVER_HEALTH_H
#define SERVER_HEALTH_H

#include <vector>
#include <random>
#include <functional>
#include <chrono>
#include <map>
#include <string>

enum class ServerState {
    HEALTHY,
    DEGRADED,
    CRITICAL,
    OFFLINE
};

class ServerHealthSimulator {
private:
    std::mt19937 rng;
    
    struct ServerHealth {
        int serverId;
        ServerState state;
        double healthScore;      // 0.0-1.0 representing server health
        double failureProbability;
        double recoveryProbability;
        std::chrono::system_clock::time_point lastStateChange;
        
        // For degraded performance calculation
        double performanceMultiplier;  // 1.0 is normal, lower values mean degraded performance
    };
    
    std::vector<ServerHealth> servers;
    std::map<ServerState, std::string> stateLabels;
    
    // Callbacks for state changes
    std::function<void(int, ServerState)> stateChangeCallback;
    std::function<void(int, double)> performanceUpdateCallback;
    
public:
    ServerHealthSimulator();
    
    // Main methods
    void addServer(int serverId);
    void removeServer(int serverId);
    void updateServerStates();
    
    // Server state management
    ServerState getServerState(int serverId) const;
    double getServerHealthScore(int serverId) const;
    double getServerPerformanceMultiplier(int serverId) const;
    std::string getServerStateLabel(int serverId) const;
    
    // Manually set server states (for testing or simulation scenarios)
    void setServerState(int serverId, ServerState state);
    void degradeServerPerformance(int serverId, double degradationFactor);
    void recoverServer(int serverId);
    
    // Callbacks
    void setStateChangeCallback(std::function<void(int, ServerState)> callback);
    void setPerformanceUpdateCallback(std::function<void(int, double)> callback);
    
    // Event simulation
    void simulateRandomFailure();
    void simulateNetworkPartition(const std::vector<int>& affectedServers);
    void simulateHighLoad(int serverId);
    
    // Utility methods
    static std::string stateToString(ServerState state);
};

#endif // SERVER_HEALTH_H