#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <random>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>
#include <map>
#include <utility>
#include <cstdlib>
#include <ctime>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <termios.h>
#endif

// Structure to represent a server
struct Server {
    int id;
    double capacity;  // Maximum load capacity
    double currentLoad;
    double utilizationRatio() const {
        return (capacity > 0) ? currentLoad / capacity : 0;
    }
};

class LoadBalancer {
private:
    std::vector<Server> servers;
    int selectedAlgorithm = 0; // 0: Round Robin, 1: Least Loaded, 2: Optimization-based
    const std::vector<std::string> algorithmNames = {
        "Round Robin",
        "Least Loaded",
        "Weighted Optimization"
    };
    int currentServerIndex = 0;
    bool running = true;
    double randomLoadAmount = 10.0;
    
    // Non-blocking key press detection
    char getKeyPress() {
        char ch = 0;
#ifdef _WIN32
        if (_kbhit()) {
            ch = _getch();
        }
#else
        struct termios old = {0};
        if (tcgetattr(0, &old) < 0) return 0;
        
        struct termios _new = old;
        _new.c_lflag &= ~ICANON;
        _new.c_lflag &= ~ECHO;
        _new.c_cc[VMIN] = 1;
        _new.c_cc[VTIME] = 0;
        
        if (tcsetattr(0, TCSANOW, &_new) < 0) return 0;
        
        fd_set rfds;
        struct timeval tv;
        FD_ZERO(&rfds);
        FD_SET(0, &rfds);
        tv.tv_sec = 0;
        tv.tv_usec = 1000;
        
        int retval = select(1, &rfds, NULL, NULL, &tv);
        if (retval > 0) {
            read(0, &ch, 1);
        }
        
        tcsetattr(0, TCSANOW, &old);
#endif
        return ch;
    }
    
    // Clear screen
    void clearScreen() {
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif
    }
    
    // Round Robin algorithm
    void distributeLoadRoundRobin(double newLoad) {
        if (servers.empty()) return;
        
        servers[currentServerIndex].currentLoad += newLoad;
        currentServerIndex = (currentServerIndex + 1) % servers.size();
    }
    
    // Least Loaded algorithm
    void distributeLoadLeastLoaded(double newLoad) {
        if (servers.empty()) return;
        
        int leastLoadedIdx = 0;
        double minUtilizationRatio = std::numeric_limits<double>::max();
        
        for (size_t i = 0; i < servers.size(); i++) {
            double ur = servers[i].utilizationRatio();
            if (ur < minUtilizationRatio) {
                minUtilizationRatio = ur;
                leastLoadedIdx = i;
            }
        }
        
        servers[leastLoadedIdx].currentLoad += newLoad;
    }
    
    // Weighted Optimization algorithm (uses a weighted approach to minimize overall variance)
    void distributeLoadOptimization(double newLoad) {
        if (servers.empty()) return;
        
        // Calculate total capacity and current utilization
        double totalCapacity = 0;
        double totalCurrentLoad = 0;
        
        for (const auto& server : servers) {
            totalCapacity += server.capacity;
            totalCurrentLoad += server.currentLoad;
        }
        
        // Calculate target utilization after adding new load
        double targetUtilization = (totalCurrentLoad + newLoad) / totalCapacity;
        
        // Calculate required load for each server to reach target utilization
        std::vector<double> targetLoads(servers.size());
        for (size_t i = 0; i < servers.size(); i++) {
            targetLoads[i] = targetUtilization * servers[i].capacity;
        }
        
        // Calculate difference between current and target loads
        std::vector<double> loadDifferences(servers.size());
        for (size_t i = 0; i < servers.size(); i++) {
            loadDifferences[i] = targetLoads[i] - servers[i].currentLoad;
        }
        
        // Distribute the new load proportionally based on the differences
        double totalPositiveDifference = 0;
        for (double diff : loadDifferences) {
            if (diff > 0) totalPositiveDifference += diff;
        }
        
        if (totalPositiveDifference > 0) {
            for (size_t i = 0; i < servers.size(); i++) {
                if (loadDifferences[i] > 0) {
                    double loadToAdd = newLoad * (loadDifferences[i] / totalPositiveDifference);
                    servers[i].currentLoad += loadToAdd;
                }
            }
        } else {
            // If all servers are above target utilization, use least loaded algorithm
            distributeLoadLeastLoaded(newLoad);
        }
    }

    // Get the total current load across all servers
    double getTotalLoad() {
        double totalLoad = 0;
        for (const auto& server : servers) {
            totalLoad += server.currentLoad;
        }
        return totalLoad;
    }

    // Increase total system load by a fixed percentage
    void increaseTotalLoad(double percentage) {
        if (servers.empty()) return;
        
        double currentTotal = getTotalLoad();
        double additionalLoad = currentTotal * (percentage / 100.0);
        
        // Add the additional load using the current algorithm
        distributeLoad(additionalLoad);
    }
    
    // Decrease total system load by a fixed percentage
    void decreaseTotalLoad(double percentage) {
        if (servers.empty()) return;
        
        double currentTotal = getTotalLoad();
        double reductionAmount = currentTotal * (percentage / 100.0);
        
        // Distribute the reduction proportionally across all servers
        for (auto& server : servers) {
            double serverReduction = reductionAmount * (server.currentLoad / currentTotal);
            server.currentLoad = std::max(0.0, server.currentLoad - serverReduction);
        }
    }

public:
    LoadBalancer(int numServers = 5) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> capacityDis(50.0, 200.0);
        
        for (int i = 0; i < numServers; i++) {
            Server server{i + 1, capacityDis(gen), 0.0};
            servers.push_back(server);
        }
        
        // Set up terminal for non-blocking input
#ifndef _WIN32
        struct termios term;
        tcgetattr(0, &term);
        term.c_lflag &= ~ICANON;
        tcsetattr(0, TCSANOW, &term);
#endif
    }
    
    ~LoadBalancer() {
        // Restore terminal
#ifndef _WIN32
        struct termios term;
        tcgetattr(0, &term);
        term.c_lflag |= ICANON;
        tcsetattr(0, TCSANOW, &term);
#endif
    }
    
    void distributeLoad(double load) {
        switch (selectedAlgorithm) {
            case 0:
                distributeLoadRoundRobin(load);
                break;
            case 1:
                distributeLoadLeastLoaded(load);
                break;
            case 2:
                distributeLoadOptimization(load);
                break;
            default:
                distributeLoadRoundRobin(load);
        }
    }
    
    void addServer() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> capacityDis(50.0, 200.0);
        
        Server server{static_cast<int>(servers.size()) + 1, capacityDis(gen), 0.0};
        servers.push_back(server);
    }
    
    void removeServer() {
        if (!servers.empty()) {
            double remainingLoad = servers.back().currentLoad;
            servers.pop_back();
            if (!servers.empty() && remainingLoad > 0) {
                distributeLoad(remainingLoad);
            }
        }
    }
    
    void addLoadToServer(int serverId, double load) {
        for (auto& server : servers) {
            if (server.id == serverId) {
                server.currentLoad += load;
                rebalance();
                break;
            }
        }
    }
    
    void rebalance() {
        // Get total load and capacity
        double totalLoad = 0;
        double totalCapacity = 0;
        
        for (const auto& server : servers) {
            totalLoad += server.currentLoad;
            totalCapacity += server.capacity;
        }
        
        // Temporary clear all loads
        for (auto& server : servers) {
            server.currentLoad = 0;
        }
        
        // Redistribute total load using the selected algorithm
        distributeLoad(totalLoad);
    }
    
    void switchAlgorithm() {
        selectedAlgorithm = (selectedAlgorithm + 1) % algorithmNames.size();
        rebalance();
    }
    
    void displayStatus() {
        clearScreen();
        
        std::cout << "===== DISTRIBUTED LOAD BALANCER SIMULATION =====" << std::endl;
        std::cout << "Current Algorithm: " << algorithmNames[selectedAlgorithm] << std::endl << std::endl;
        
        std::cout << "Server Status:" << std::endl;
        std::cout << "-----------------------------------------------------------------" << std::endl;
        std::cout << "| ID |  Capacity  |   Load    | Utilization |       Status Bar       |" << std::endl;
        std::cout << "-----------------------------------------------------------------" << std::endl;
        
        for (const auto& server : servers) {
            double utilizationPercentage = server.utilizationRatio() * 100;
            int barLength = static_cast<int>(utilizationPercentage / 5);
            
            std::string statusBar = "[";
            for (int i = 0; i < 20; i++) {
                if (i < barLength) {
                    // Color coding based on utilization
                    if (utilizationPercentage < 50) {
                        statusBar += "#"; // Green in terminal
                    } else if (utilizationPercentage < 80) {
                        statusBar += "="; // Yellow in terminal
                    } else {
                        statusBar += "!"; // Red in terminal
                    }
                } else {
                    statusBar += " ";
                }
            }
            statusBar += "]";
            
            std::cout << "| " << std::setw(2) << server.id << " | " 
                      << std::setw(10) << std::fixed << std::setprecision(2) << server.capacity << " | "
                      << std::setw(9) << std::fixed << std::setprecision(2) << server.currentLoad << " | "
                      << std::setw(10) << std::fixed << std::setprecision(2) << utilizationPercentage << "% | "
                      << statusBar << " |" << std::endl;
        }
        std::cout << "-----------------------------------------------------------------" << std::endl;
        
        // Calculate system statistics
        double totalCapacity = 0;
        double totalLoad = 0;
        double maxUtilization = 0;
        double minUtilization = 1.0;
        double avgUtilization = 0;
        
        for (const auto& server : servers) {
            totalCapacity += server.capacity;
            totalLoad += server.currentLoad;
            maxUtilization = std::max(maxUtilization, server.utilizationRatio());
            minUtilization = std::min(minUtilization, server.utilizationRatio());
            avgUtilization += server.utilizationRatio();
        }
        
        if (!servers.empty()) {
            avgUtilization /= servers.size();
            minUtilization = (!servers.empty()) ? minUtilization : 0.0;
        } else {
            maxUtilization = 0.0;
            minUtilization = 0.0;
        }
        
        double systemUtilization = (totalCapacity > 0) ? (totalLoad / totalCapacity) * 100 : 0;
        double loadImbalance = maxUtilization - minUtilization;
        
        std::cout << std::endl;
        std::cout << "System Statistics:" << std::endl;
        std::cout << "Total Capacity: " << std::fixed << std::setprecision(2) << totalCapacity << std::endl;
        std::cout << "Total Load: " << std::fixed << std::setprecision(2) << totalLoad << std::endl;
        std::cout << "System Utilization: " << std::fixed << std::setprecision(2) << systemUtilization << "%" << std::endl;
        std::cout << "Load Imbalance: " << std::fixed << std::setprecision(2) << (loadImbalance * 100) << "%" << std::endl;
        
        std::cout << std::endl;
        std::cout << "Commands:" << std::endl;
        std::cout << "  a: Add random load        s: Add server" << std::endl;
        std::cout << "  d: Remove server          r: Rebalance" << std::endl;
        std::cout << "  m: Switch algorithm       1-9: Add load to server" << std::endl;
        std::cout << "  +: Increase total system load by 10%" << std::endl;
        std::cout << "  -: Decrease total system load by 10%" << std::endl;
        std::cout << "  q: Quit" << std::endl;
    }
    
    void run() {
        while (running) {
            displayStatus();
            
            char key = getKeyPress();
            
            switch (key) {
                case 'a': {
                    // Add random load
                    std::random_device rd;
                    std::mt19937 gen(rd());
                    std::uniform_real_distribution<> loadDis(1.0, randomLoadAmount);
                    double load = loadDis(gen);
                    distributeLoad(load);
                    break;
                }
                case 's':
                    // Add server
                    addServer();
                    break;
                case 'd':
                    // Remove server
                    removeServer();
                    break;
                case 'r':
                    // Rebalance
                    rebalance();
                    break;
                case 'm':
                    // Switch algorithm
                    switchAlgorithm();
                    break;
                case '+':
                    // Increase total system load by 10%
                    increaseTotalLoad(10.0);
                    break;
                case '-':
                    // Decrease total system load by 10%
                    decreaseTotalLoad(10.0);
                    break;
                case 'q':
                    // Quit
                    running = false;
                    break;
                default:
                    // Check for numeric keys to add load to specific servers
                    if (key >= '1' && key <= '9') {
                        int serverId = key - '0';
                        if (serverId <= static_cast<int>(servers.size())) {
                            std::random_device rd;
                            std::mt19937 gen(rd());
                            std::uniform_real_distribution<> loadDis(5.0, 20.0);
                            double load = loadDis(gen);
                            
                            addLoadToServer(serverId, load);
                        }
                    }
                    break;
            }
            
            // Add some small random fluctuations to loads
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<> fluctuationDis(-2.0, 2.0);
            std::uniform_int_distribution<> serverDis(0, servers.size() - 1);
            
            if (!servers.empty()) {
                int randomServerId = serverDis(gen);
                double fluctuation = fluctuationDis(gen);
                servers[randomServerId].currentLoad = std::max(0.0, servers[randomServerId].currentLoad + fluctuation);
            }
            
            // Slight delay for visualizing changes
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
};

int main() {
    std::srand(std::time(nullptr));
    
    std::cout << "Starting Distributed Load Balancer Simulation..." << std::endl;
    std::cout << "Press any key to continue..." << std::endl;
    std::cin.get();
    
    LoadBalancer balancer(5);
    balancer.run();
    
    std::cout << "Simulation ended. Thank you!" << std::endl;
    return 0;
}