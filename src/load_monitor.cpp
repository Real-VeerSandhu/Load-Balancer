#include "include/load_monitor.h"
#include <iostream>
#include <numeric>
#include <iomanip>
#include <sstream>

LoadMonitor::LoadMonitor(const std::string& logFilePath) : currentAlgorithm("Round Robin") {
    logFile.open(logFilePath, std::ios::out | std::ios::app);
    startTime = std::chrono::system_clock::now();
    
    if (logFile.is_open()) {
        logFile << "=== Load Balancer Monitoring Started at " 
                << std::put_time(std::localtime(std::chrono::system_clock::to_time_t(startTime)), 
                                 "%Y-%m-%d %H:%M:%S") << " ===" << std::endl;
        logFile << "Timestamp,Algorithm,ServerCount,AvgLoad,LoadVariance,ResponseTime" << std::endl;
    } else {
        std::cerr << "Warning: Could not open log file for monitoring!" << std::endl;
    }
}

LoadMonitor::~LoadMonitor() {
    if (logFile.is_open()) {
        logFile << "=== Monitoring Ended after " << getElapsedTimeSeconds() 
                << " seconds ===" << std::endl;
        logFile.close();
    }
}

void LoadMonitor::recordMetrics(const std::vector<int>& serverLoads, double responseTime) {
    double timestamp = getElapsedTimeSeconds();
    double avgLoad = calculateAverageLoad(serverLoads);
    double variance = calculateLoadVariance(serverLoads);
    
    // Store metrics
    MetricsSnapshot snapshot = {
        timestamp,
        avgLoad,
        variance,
        responseTime,
        static_cast<int>(serverLoads.size()),
        currentAlgorithm
    };
    metrics.push_back(snapshot);
    
    // Log to file
    if (logFile.is_open()) {
        logFile << timestamp << ","
                << currentAlgorithm << ","
                << serverLoads.size() << ","
                << avgLoad << ","
                << variance << ","
                << responseTime << std::endl;
    }
}

void LoadMonitor::setAlgorithm(const std::string& algorithm) {
    currentAlgorithm = algorithm;
    
    if (logFile.is_open()) {
        logFile << getElapsedTimeSeconds() << ",Algorithm changed to: " 
                << algorithm << std::endl;
    }
}

double LoadMonitor::calculateLoadVariance(const std::vector<int>& serverLoads) {
    if (serverLoads.empty()) return 0.0;
    
    double avg = calculateAverageLoad(serverLoads);
    double variance = 0.0;
    
    for (int load : serverLoads) {
        variance += (load - avg) * (load - avg);
    }
    
    return variance / serverLoads.size();
}

double LoadMonitor::calculateAverageLoad(const std::vector<int>& serverLoads) {
    if (serverLoads.empty()) return 0.0;
    
    double sum = std::accumulate(serverLoads.begin(), serverLoads.end(), 0);
    return sum / serverLoads.size();
}

double LoadMonitor::getElapsedTimeSeconds() {
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration<double>(now - startTime).count();
}

void LoadMonitor::logServerAddition() {
    if (logFile.is_open()) {
        logFile << getElapsedTimeSeconds() << ",Server added" << std::endl;
    }
}

void LoadMonitor::logServerRemoval() {
    if (logFile.is_open()) {
        logFile << getElapsedTimeSeconds() << ",Server removed" << std::endl;
    }
}

void LoadMonitor::logRebalancing() {
    if (logFile.is_open()) {
        logFile << getElapsedTimeSeconds() << ",Load rebalanced" << std::endl;
    }
}

void LoadMonitor::generateReport(const std::string& reportPath) {
    std::ofstream report(reportPath);
    if (!report.is_open()) {
        std::cerr << "Failed to create performance report file!" << std::endl;
        return;
    }
    
    report << "=== LOAD BALANCER PERFORMANCE REPORT ===" << std::endl;
    report << "Total runtime: " << getElapsedTimeSeconds() << " seconds" << std::endl;
    report << "Current algorithm: " << currentAlgorithm << std::endl;
    report << "Number of metrics recorded: " << metrics.size() << std::endl << std::endl;
    
    // Calculate averages by algorithm
    std::map<std::string, std::vector<MetricsSnapshot>> algorithmMetrics;
    for (const auto& snapshot : metrics) {
        algorithmMetrics[snapshot.algorithm].push_back(snapshot);
    }
    
    report << "PERFORMANCE BY ALGORITHM:" << std::endl;
    report << "--------------------------" << std::endl;
    
    for (const auto& pair : algorithmMetrics) {
        double avgVariance = 0.0;
        double avgResponse = 0.0;
        
        for (const auto& snapshot : pair.second) {
            avgVariance += snapshot.loadVariance;
            avgResponse += snapshot.responseTime;
        }
        
        avgVariance /= pair.second.size();
        avgResponse /= pair.second.size();
        
        report << "Algorithm: " << pair.first << std::endl;
        report << "  Samples: " << pair.second.size() << std::endl;
        report << "  Avg Load Variance: " << avgVariance << std::endl;
        report << "  Avg Response Time: " << avgResponse << " ms" << std::endl << std::endl;
    }
    
    report << "=== END OF REPORT ===" << std::endl;
    report.close();
    
    std::cout << "Performance report generated at: " << reportPath << std::endl;
}

std::string LoadMonitor::getPerformanceSummary() {
    std::stringstream summary;
    summary << "Performance Summary:" << std::endl;
    summary << "- Current Algorithm: " << currentAlgorithm << std::endl;
    
    if (!metrics.empty()) {
        auto latest = metrics.back();
        summary << "- Current Avg Load: " << latest.avgLoad << std::endl;
        summary << "- Current Load Variance: " << latest.loadVariance << std::endl;
        summary << "- Current Response Time: " << latest.responseTime << " ms" << std::endl;
    }
    
    return summary.str();
}