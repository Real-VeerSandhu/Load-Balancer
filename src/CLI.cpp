#include "CLI.h"
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include <thread>
#include <chrono>

CLI::CLI(LoadBalancer& lb) : loadBalancer(lb) {
    // Create and start monitor (using new for C++11 compatibility)
    monitor.reset(new Monitor(loadBalancer, 500, 0.3, 5));
    monitor->start();
}

void CLI::run() {
    // Initial display
    monitor->refreshDisplay();
    
    std::string line;
    while (true) {
        // Pause monitor updates while reading input
        monitor->pauseUpdates();
        std::cout << "> " << std::flush;
        std::getline(std::cin, line);
        monitor->resumeUpdates();
        
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
            monitor->pauseUpdates();
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
            monitor->pauseUpdates();
            std::cout << "Unknown command: " << command << std::endl;
            std::cout << "Type 'help' for available commands." << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(1500));
            monitor->resumeUpdates();
        }
    }
}

void CLI::handleAdd(const std::string& args) {
    monitor->pauseUpdates();
    if (args.empty()) {
        std::cout << "Usage: add <power>" << std::endl;
        std::cout << "Example: add 50" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        monitor->resumeUpdates();
        return;
    }
    
    int power = parseInteger(args, "add");
    if (power <= 0) {
        std::cout << "Error: Power must be a positive integer." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        monitor->resumeUpdates();
        return;
    }
    
    int id = loadBalancer.addServer(power);
    std::cout << "Added server with ID " << id << " and power " << power << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    monitor->resumeUpdates();
}

void CLI::handleRemove(const std::string& args) {
    monitor->pauseUpdates();
    if (args.empty()) {
        std::cout << "Usage: remove <server_id>" << std::endl;
        std::cout << "Example: remove 3" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        monitor->resumeUpdates();
        return;
    }
    
    int id = parseInteger(args, "remove");
    if (id <= 0) {
        std::cout << "Error: Server ID must be a positive integer." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        monitor->resumeUpdates();
        return;
    }
    
    if (loadBalancer.removeServer(id)) {
        std::cout << "Removed server with ID " << id << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    } else {
        std::cout << "Error: Server with ID " << id << " not found." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    }
    monitor->resumeUpdates();
}

void CLI::handleSend(const std::string& args) {
    monitor->pauseUpdates();
    if (args.empty()) {
        std::cout << "Usage: send <request_count>" << std::endl;
        std::cout << "Example: send 200" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        monitor->resumeUpdates();
        return;
    }
    
    int count = parseInteger(args, "send");
    if (count <= 0) {
        std::cout << "Error: Request count must be a positive integer." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        monitor->resumeUpdates();
        return;
    }
    
    if (loadBalancer.getServerCount() == 0) {
        std::cout << "Error: No servers available. Add servers first." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        monitor->resumeUpdates();
        return;
    }
    
    loadBalancer.distributeRequests(count);
    std::cout << "Distributed " << count << " requests." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(800));
    monitor->resumeUpdates();
}

void CLI::handleList() {
    // List just refreshes the display
    monitor->refreshDisplay();
}

void CLI::handleHelp() {
    monitor->pauseUpdates();
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
    std::cout << "Press Enter to continue..." << std::flush;
    std::string dummy;
    std::getline(std::cin, dummy);
    monitor->resumeUpdates();
}

void CLI::handleReset() {
    monitor->pauseUpdates();
    loadBalancer.resetAllLoads();
    std::cout << "All server loads have been reset." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    monitor->resumeUpdates();
}

int CLI::parseInteger(const std::string& str, const std::string& command) {
    try {
        return std::stoi(str);
    } catch (const std::exception& e) {
        std::cout << "Error: Invalid number format for " << command << " command." << std::endl;
        return -1;
    }
}

