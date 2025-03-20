#ifndef LOAD_MONITOR
#define LOAD_MONITOR

#include <vector>
#include <string>
#include <fstream>
#include <chrono>
#include <cmath>

class LoadMonitor {
private:
    std::ofstream logFile;
    std::chrono::time_point<std::chrono::system_clock> startTime;
    std::string currentAlgorithm;
    
    // Metrics storage
    struct MetricsSnapshot {
        double timestamp;
        double avgLoad;
        double loadVariance;
        double responseTime;
        int serverCount;
        std::string algorithm;
    };
    
    std::vector<MetricsSnapshot> metrics;
    
public:
    LoadMonitor(const std::string& logFilePath = "load_balancer_metrics.log");
    ~LoadMonitor();
    
    // Core monitoring methods
    void recordMetrics(const std::vector<int>& serverLoads, double responseTime);
    void setAlgorithm(const std::string& algorithm);
    void logServerAddition();
    void logServerRemoval();
    void logRebalancing();
    
    // Analysis methods
    double calculateLoadVariance(const std::vector<int>& serverLoads);
    double calculateAverageLoad(const std::vector<int>& serverLoads);
    double getElapsedTimeSeconds();
    
    // Report generation
    void generateReport(const std::string& reportPath = "performance_report.txt");
    std::string getPerformanceSummary();
};

#endif