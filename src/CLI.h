#ifndef CLI_H
#define CLI_H

#include "LoadBalancer.h"
#include "Monitor.h"
#include <string>
#include <memory>

class CLI {
private:
    LoadBalancer& loadBalancer;
    std::unique_ptr<Monitor> monitor;
    
    // Command handlers
    void handleAdd(const std::string& args);
    void handleRemove(const std::string& args);
    void handleSend(const std::string& args);
    void handleList();
    void handleHelp();
    void handleReset();
    
    // Utility
    int parseInteger(const std::string& str, const std::string& command);

public:
    CLI(LoadBalancer& lb);
    void run();
};

#endif // CLI_H

