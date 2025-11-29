#include "CLI.h"
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include <thread>
#include <chrono>

CLI::CLI(LoadBalancer& lb) : loadBalancer(lb) {
    // Create and start monitor
    monitor = std::make_unique<Monitor>(loadBalancer, 500, 0.3, 5);
    monitor->start();
}

void CLI::run() {
    std::string line;
    while (true) {
        // Monitor handles display, we just need to get input
        // The prompt is shown by monitor, so we read directly
        std::getline(std::cin, line);
        
        if (line.empty()) {
            continue;
        }
        
        // Convert to lowercase for case-insensitive commands
        std::string lowerLine = line;
        std::transform(lowerLine.begin(), lowerLine.end(), lowerLine.begin(), ::tolower);
        
        // Tokenize
        std::istringstream iss(lowerLine);
        std::string command;
        iss >> command;
        
        // Get remaining arguments
        std::string args;
        std::getline(iss, args);
        // Trim leading whitespace
        if (!args.empty() && args[0] == ' ') {
            args = args.substr(1);
        }
        
        if (command == "quit" || command == "exit" || command == "q") {
            monitor->stop();
            std::cout << "\nExiting..." << std::endl;
            break;
        }
        else if (command == "add") {
            handleAdd(args);
        }
        else if (command == "remove") {
            handleRemove(args);
        }
        else if (command == "send") {
            handleSend(args);
        }
        else if (command == "list" || command == "ls") {
            handleList();
        }
        else if (command == "help" || command == "h") {
            handleHelp();
        }
        else if (command == "reset") {
            handleReset();
        }
        else {
            std::cout << "Unknown command: " << command << std::endl;
            std::cout << "Type 'help' for available commands." << std::endl;
        }
    }
}

void CLI::handleAdd(const std::string& args) {
    if (args.empty()) {
        std::cout << "Usage: add <power>" << std::endl;
        std::cout << "Example: add 50" << std::endl;
        return;
    }
    
    int power = parseInteger(args, "add");
    if (power <= 0) {
        std::cout << "Error: Power must be a positive integer." << std::endl;
        return;
    }
    
    int id = loadBalancer.addServer(power);
    monitor->refreshDisplay();
    // Message will be shown briefly before monitor updates
}

void CLI::handleRemove(const std::string& args) {
    if (args.empty()) {
        std::cout << "Usage: remove <server_id>" << std::endl;
        std::cout << "Example: remove 3" << std::endl;
        return;
    }
    
    int id = parseInteger(args, "remove");
    if (id <= 0) {
        std::cout << "Error: Server ID must be a positive integer." << std::endl;
        return;
    }
    
    if (loadBalancer.removeServer(id)) {
        monitor->refreshDisplay();
    } else {
        // Error message - monitor will refresh on next cycle
        std::cout << "\nError: Server with ID " << id << " not found." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        monitor->refreshDisplay();
    }
}

void CLI::handleSend(const std::string& args) {
    if (args.empty()) {
        std::cout << "Usage: send <request_count>" << std::endl;
        std::cout << "Example: send 200" << std::endl;
        return;
    }
    
    int count = parseInteger(args, "send");
    if (count <= 0) {
        std::cout << "Error: Request count must be a positive integer." << std::endl;
        return;
    }
    
    if (loadBalancer.getServerCount() == 0) {
        std::cout << "Error: No servers available. Add servers first." << std::endl;
        return;
    }
    
    loadBalancer.distributeRequests(count);
    monitor->refreshDisplay();
}

void CLI::handleList() {
    monitor->refreshDisplay();
}

void CLI::handleHelp() {
    std::cout << "\nAvailable commands:" << std::endl;
    std::cout << "  add <power>        - Add a server with specified power" << std::endl;
    std::cout << "                      Example: add 50" << std::endl;
    std::cout << "  remove <id>        - Remove a server by ID" << std::endl;
    std::cout << "                      Example: remove 3" << std::endl;
    std::cout << "  send <count>       - Distribute requests to servers" << std::endl;
    std::cout << "                      Example: send 200" << std::endl;
    std::cout << "  list               - Show all servers and their states" << std::endl;
    std::cout << "  reset              - Reset all server loads to zero" << std::endl;
    std::cout << "  help               - Show this help message" << std::endl;
    std::cout << "  quit               - Exit the simulator\n" << std::endl;
}

void CLI::handleReset() {
    loadBalancer.resetAllLoads();
    monitor->refreshDisplay();
}

int CLI::parseInteger(const std::string& str, const std::string& command) {
    try {
        return std::stoi(str);
    } catch (const std::exception& e) {
        std::cout << "Error: Invalid number format for " << command << " command." << std::endl;
        return -1;
    }
}

