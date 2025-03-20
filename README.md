# Distributed Load Balancer with Optimization Algorithms

## Project Overview
This project simulates a distributed load balancing system with multiple optimization strategies–*Round Robin*, *Least Loaded*, and *Weight Optimized*

The simulation allows users to observe and interact with different load balancing algorithms, monitor system performance metrics, and study how various optimization techniques impact resource utilization across a network of servers.

## Technical Highlights

### Distributed Systems Architecture
- **Multi-Server Environment**: Simulates a distributed network of servers with varying capacities
- **Dynamic Resource Allocation**: Implements real-time redistribution of workloads across available resources
- **Server Scaling**: Enables runtime addition and removal of servers with automatic load rebalancing
- **System Adaptability**: Demonstrates resilience to changing network conditions and shifting workloads

### Optimization Mathematics Implementation
- **Weighted Optimization Algorithm**: Utilizes mathematical optimization techniques to minimize variance in server utilization
- **Resource Utilization Minimization**: Implements algorithms that optimize for balanced capacity utilization
- **Load Imbalance Reduction**: Calculates and displays system-wide imbalance metrics in real time
- **Non-Linear Load Redistribution**: Employs non-linear optimization to determine optimal resource allocation

### Algorithm Portfolio
1. **Round Robin Algorithm**
   - Cyclic allocation strategy
   - O(1) complexity for load assignment decisions
   - Baseline implementation for comparison purposes

2. **Least Loaded Algorithm**
   - Greedy optimization strategy focused on minimizing peak utilization
   - O(n) complexity with utilization ratio calculations
   - Demonstrates simple but effective resource allocation

3. **Weighted Optimization Algorithm**
   - Implementation utilizing mathematical optimization principles
   - Calculates target utilization ratios proportional to server capacities
   - Minimizes system-wide variance using proportional distribution calculations
   - Demonstrates practical application of distribution optimization theory

### Real-Time Visualization and Analytics
- **Dynamic ASCII Visualization**: Renders server load distributions with utilization indicators
- **Performance Metrics**: Calculates and displays key system statistics:
  - Total system capacity and current load
  - System-wide utilization percentage
  - Load imbalance across servers
  - Per-server resource utilization


## Architecture
The architecture of this load balancing system is designed with modularity, flexibility, and scalability in mind. Several object-oriented design patterns, along with adherence to SOLID principles, contribute to a robust and maintainable structure. 

- **Composition Pattern**: The LoadBalancer class contains and manages a collection of Server objects, establishing a strong "has-a" relationship. The LoadBalancer controls the lifecycle of servers, ensuring that they are created and deleted within its context.
- **Strategy Pattern**: The system supports three interchangeable load distribution algorithms—Round Robin, Least Loaded, and Weighted Optimization. These strategies are decoupled from their execution, allowing users to switch between them at runtime without disrupting the system. Adding new algorithms is straightforward through the implementation of additional distribution methods.
- **Observer Pattern**: The UI display functions as an observer of the system state, automatically updating to reflect changes in server loads. This pattern ensures a clear separation between the data model (servers and their loads) and the presentation layer, maintaining modularity.
- **Command Pattern**: User inputs are translated into specific actions through a command interface. Each key press corresponds to a command executed by the system. The decoupling of commands from their implementation allows for easy extension of the system’s functionality.

## Usage

To compile the project, run the following command from the root directory: 
```
make
```

This will compile the source files in the `/src` directory and create the executable. The header files are located in the `/include` directory. To clean up object files and the executable, use:
```
make clean
```

To rebuild the project from scratch, use: 
```
make clean
```
