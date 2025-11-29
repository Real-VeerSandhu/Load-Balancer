#ifndef MONITOR_H
#define MONITOR_H

#include "LoadBalancer.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <vector>

class Monitor {
private:
    LoadBalancer& loadBalancer;
    std::atomic<bool> running;
    std::thread monitorThread;
    std::mutex displayMutex;
    
    // Configuration
    int updateIntervalMs;  // Update interval in milliseconds
    double fluctuationRate; // Rate of random load changes (0.0 to 1.0)
    int maxFluctuationAmount; // Maximum amount of load to add/remove randomly
    
    // Internal methods
    void monitorLoop();
    void applyRandomFluctuations();
    void clearScreen();
    void displayServerStates();
    std::vector<ServerSnapshot> getServerSnapshots(); // ServerSnapshot defined in LoadBalancer.h
    void drawStatusBar(int totalServers, int totalLoad, int totalPower);

public:
    Monitor(LoadBalancer& lb, int updateIntervalMs = 500, 
            double fluctuationRate = 0.3, int maxFluctuationAmount = 5);
    ~Monitor();
    
    // Control
    void start();
    void stop();
    bool isRunning() const;
    
    // Configuration
    void setUpdateInterval(int ms);
    void setFluctuationRate(double rate);
    void setMaxFluctuationAmount(int amount);
    
    // Display control
    void refreshDisplay();
};

#endif // MONITOR_H

