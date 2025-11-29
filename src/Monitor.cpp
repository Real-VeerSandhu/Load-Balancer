#include "Monitor.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <random>
#include <algorithm>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#endif

Monitor::Monitor(LoadBalancer& lb, int updateIntervalMs, 
                 double fluctuationRate, int maxFluctuationAmount)
    : loadBalancer(lb), running(false), updateIntervalMs(updateIntervalMs),
      fluctuationRate(fluctuationRate), maxFluctuationAmount(maxFluctuationAmount) {
}

Monitor::~Monitor() {
    stop();
}

void Monitor::start() {
    if (running) return;
    
    running = true;
    monitorThread = std::thread(&Monitor::monitorLoop, this);
}

void Monitor::stop() {
    if (!running) return;
    
    running = false;
    if (monitorThread.joinable()) {
        monitorThread.join();
    }
}

bool Monitor::isRunning() const {
    return running;
}

void Monitor::setUpdateInterval(int ms) {
    updateIntervalMs = ms;
}

void Monitor::setFluctuationRate(double rate) {
    fluctuationRate = std::max(0.0, std::min(1.0, rate));
}

void Monitor::setMaxFluctuationAmount(int amount) {
    maxFluctuationAmount = std::max(1, amount);
}

void Monitor::monitorLoop() {
    while (running) {
        std::lock_guard<std::mutex> lock(displayMutex);
        
        // Apply random fluctuations
        applyRandomFluctuations();
        
        // Update display
        clearScreen();
        displayServerStates();
        
        // Sleep for update interval
        std::this_thread::sleep_for(std::chrono::milliseconds(updateIntervalMs));
    }
}

void Monitor::applyRandomFluctuations() {
    if (loadBalancer.getServerCount() == 0) return;
    
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<> probDist(0.0, 1.0);
    static std::uniform_int_distribution<> amountDist(-maxFluctuationAmount, maxFluctuationAmount);
    
    // Apply fluctuations to random servers based on fluctuation rate
    int serverCount = loadBalancer.getServerCount();
    int serversToFluctuate = static_cast<int>(serverCount * fluctuationRate);
    
    if (serversToFluctuate == 0 && probDist(gen) < fluctuationRate) {
        serversToFluctuate = 1;
    }
    
    for (int i = 0; i < serversToFluctuate; ++i) {
        if (probDist(gen) < fluctuationRate) {
            int fluctuation = amountDist(gen);
            if (fluctuation != 0) {
                // Apply fluctuation through load balancer
                loadBalancer.applyRandomFluctuation(fluctuation);
            }
        }
    }
}

void Monitor::clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    // ANSI escape codes for clearing screen and moving cursor to top
    std::cout << "\033[2J\033[H";
#endif
}

void Monitor::displayServerStates() {
    auto snapshots = getServerSnapshots();
    
    if (snapshots.empty()) {
        std::cout << "\n=== Load Balancer Simulator ===" << std::endl;
        std::cout << "No servers configured. Use 'add <power>' to add servers.\n" << std::endl;
        return;
    }
    
    // Calculate totals
    int totalServers = snapshots.size();
    int totalLoad = 0;
    int totalPower = 0;
    for (const auto& snap : snapshots) {
        totalLoad += snap.currentLoad;
        totalPower += snap.power;
    }
    
    // Header
    std::cout << "\n=== Load Balancer Simulator - Real-Time Monitor ===" << std::endl;
    drawStatusBar(totalServers, totalLoad, totalPower);
    
    // Table header
    std::cout << "\n" << std::string(70, '-') << std::endl;
    std::cout << std::left 
              << std::setw(8) << "ID" 
              << std::setw(12) << "Power" 
              << std::setw(12) << "Load" 
              << std::setw(15) << "Load %"
              << std::setw(20) << "Visual"
              << std::endl;
    std::cout << std::string(70, '-') << std::endl;
    
    // Server rows
    for (const auto& snap : snapshots) {
        std::cout << std::left 
                  << std::setw(8) << snap.id
                  << std::setw(12) << snap.power
                  << std::setw(12) << snap.currentLoad
                  << std::fixed << std::setprecision(2)
                  << std::setw(15) << snap.loadPercentage << "%";
        
        // Visual bar
        int barLength = 20;
        int filled = static_cast<int>((snap.loadPercentage / 100.0) * barLength);
        filled = std::min(filled, barLength);
        filled = std::max(0, filled);
        
        std::cout << "[";
        for (int i = 0; i < barLength; ++i) {
            if (i < filled) {
                if (snap.loadPercentage > 80) {
                    std::cout << "\033[31m#\033[0m"; // Red for high load
                } else if (snap.loadPercentage > 50) {
                    std::cout << "\033[33m#\033[0m"; // Yellow for medium load
                } else {
                    std::cout << "\033[32m#\033[0m"; // Green for low load
                }
            } else {
                std::cout << " ";
            }
        }
        std::cout << "]";
        std::cout << std::endl;
    }
    
    std::cout << std::string(70, '-') << std::endl;
    std::cout << "\nCommands: add <power> | remove <id> | send <count> | reset | help | quit\n" << std::endl;
    std::cout << "> " << std::flush;
}

void Monitor::drawStatusBar(int totalServers, int totalLoad, int totalPower) {
    double avgLoadPercentage = totalPower > 0 ? (static_cast<double>(totalLoad) / totalPower) * 100.0 : 0.0;
    
    std::cout << "Servers: " << totalServers 
              << " | Total Load: " << totalLoad 
              << " | Total Power: " << totalPower
              << " | Avg Load: " << std::fixed << std::setprecision(1) << avgLoadPercentage << "%"
              << std::endl;
}

std::vector<ServerSnapshot> Monitor::getServerSnapshots() {
    return loadBalancer.getServerSnapshots();
}

void Monitor::refreshDisplay() {
    std::lock_guard<std::mutex> lock(displayMutex);
    clearScreen();
    displayServerStates();
}

