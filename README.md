# Load Balancer Simulator

A C++ command-line simulator for load balancing algorithms, demonstrating how requests are distributed across multiple backend servers with varying processing capacities.

## Features

- **Server Management**: Add and remove servers with configurable power/weight
- **Multiple Load Balancing Strategies**:
  - Weighted Round Robin (default)
  - Least Loaded
  - Round Robin
  - Random
- **Request Distribution**: Simulate request distribution across servers
- **Interactive CLI**: Simple command-line interface for managing the simulation
- **Load Monitoring**: View server states including power, current load, and load percentage

## Project Structure

```
loadbalance_fix/
├── src/
│   ├── main.cpp
│   ├── Server.h / Server.cpp
│   ├── LoadBalancer.h / LoadBalancer.cpp
│   ├── RequestGenerator.h / RequestGenerator.cpp
│   └── CLI.h / CLI.cpp
├── build/          (generated)
├── Makefile
└── README.md
```

## Building

### Prerequisites
- C++11 compatible compiler (g++, clang++, etc.)
- Make

### Compilation

```bash
make
```

This will create the executable `sim` in the project root.

### Clean Build Artifacts

```bash
make clean
```

## Usage

Run the simulator:

```bash
./sim
```

### Available Commands

- `add <power>` - Add a server with specified power
  - Example: `add 50`
  
- `remove <id>` - Remove a server by ID
  - Example: `remove 3`
  
- `send <count>` - Distribute requests to servers
  - Example: `send 200`
  
- `list` - Show all servers and their current states
  
- `reset` - Reset all server loads to zero
  
- `help` - Show available commands
  
- `quit` - Exit the simulator

### Example Session

```
> add 50
Added server with ID 1 and power 50

> add 30
Added server with ID 2 and power 30

> add 20
Added server with ID 3 and power 20

> send 200
Distributing 200 requests...

------------------------------------------------------------
ID       Power       Load         Load %        
------------------------------------------------------------
1        50          100         200.00%       
2        30          60          200.00%       
3        20          40          200.00%       
------------------------------------------------------------

> list
------------------------------------------------------------
ID       Power       Load         Load %        
------------------------------------------------------------
1        50          100         200.00%       
2        30          60          200.00%       
3        20          40          200.00%       
------------------------------------------------------------

> reset
All server loads have been reset.

> quit
Exiting...
```

## Architecture

### Server Module
Represents a backend server with:
- Unique ID
- Power (processing capacity/weight)
- Current load tracking

### LoadBalancer Module
Central router that:
- Manages a collection of servers
- Distributes requests using configurable strategies
- Provides server state visualization

### RequestGenerator Module
Simple request generation (currently returns count directly, extensible for future patterns).

### CLI Module
Interactive command-line interface that:
- Parses user commands
- Executes load balancer operations
- Displays results

## Load Balancing Strategies

### Weighted Round Robin (Default)
Distributes requests proportionally based on server power. Servers with higher power receive more requests.

### Least Loaded
Always routes requests to the server with the lowest current load.

### Round Robin
Distributes requests sequentially, cycling through servers in order.

### Random
Randomly selects a server for each request.

## Future Enhancements

Potential extensions:
- Multiple load balancing strategies selected at runtime
- ASCII bar charts for visual load representation
- Request logging and metrics (RPS)
- Simulated server outages/failures
- Dynamic load decay over time
- Configuration file support (JSON) for initial server setup
- Health checks and automatic failover

## License

This is a learning project. Feel free to use and modify as needed.

