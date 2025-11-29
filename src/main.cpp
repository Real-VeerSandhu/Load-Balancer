#include "CLI.h"
#include "LoadBalancer.h"
#include <iostream>

int main() {
    // Create load balancer with default strategy (Weighted Round Robin)
    LoadBalancer lb(LoadBalanceStrategy::WEIGHTED_ROUND_ROBIN);
    
    // Create and run CLI
    CLI cli(lb);
    cli.run();
    
    return 0;
}

