#include "include/load_monitor.h"
#include "include/load_balancer.h"
#include "include/server_health.h"

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

void displayStatus() {
    system("clear"); // Or system("cls") on Windows
    
    std::cout << "===========================================" << std::endl;
    std::cout << "    DISTRIBUTED LOAD BALANCER SIMULATION   " << std::endl;
    std::cout << "===========================================" << std::endl;
    
    std::cout << "Current Strategy: " << getStrategyName() << std::endl;
    std::cout << std::endl;
    
    std::cout << "SERVER STATUS:" << std::endl;
    std::cout << "------------------------------------------" << std::endl;
    for (const auto& server : servers) {
        double utilization = server.getUtilization();
        
        std::cout << "Server #" << server.id << ": ";
        std::cout << "Capacity: " << std::fixed << std::setprecision(1) << server.capacity << " | ";
        std::cout << "Load: " << std::fixed << std::setprecision(1) << (utilization * 100.0) << "% | ";
        std::cout << "Queue: " << server.getQueueSize() << " | ";
        std::cout << "Avg Latency: " << std::fixed << std::setprecision(3) << server.getAverageLatency() << "s";
        std::cout << std::endl;
    }
    
    std::cout << std::endl;
    
    std::cout << "SYSTEM STATISTICS:" << std::endl;
    std::cout << "------------------------------------------" << std::endl;
    std::cout << "Arrival Rate: " << std::fixed << std::setprecision(1) << arrivalRate << " requests/second" << std::endl;
    std::cout << "Total Processed Requests: " << getTotalProcessedRequests() << std::endl;
    std::cout << "Average System Latency: " << std::fixed << std::setprecision(3) << getAverageSystemLatency() << "s" << std::endl;
    std::cout << "Average Utilization: " << std::fixed << std::setprecision(1) << (getAverageUtilization() * 100.0) << "%" << std::endl;
    
    std::cout << std::endl;
    
    std::cout << "INTERACTIVE CONTROLS:" << std::endl;
    std::cout << "------------------------------------------" << std::endl;
    std::cout << "1: Round Robin" << std::endl;
    std::cout << "2: Least Connections" << std::endl;
    std::cout << "3: Optimization Based" << std::endl;
    std::cout << "+: Increase arrival rate" << std::endl;
    std::cout << "-: Decrease arrival rate" << std::endl;
    std::cout << "q: Quit simulation" << std::endl;
}


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
