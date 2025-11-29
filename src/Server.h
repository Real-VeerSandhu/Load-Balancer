#ifndef SERVER_H
#define SERVER_H

class Server {
private:
    int id;
    int power;
    int currentLoad;

public:
    // Constructor
    Server(int serverId, int serverPower);
    
    // Load management
    void addLoad(int amount);
    bool canHandle(int amount) const;
    void resetLoad();
    
    // Getters
    int getId() const;
    int getPower() const;
    int getCurrentLoad() const;
    
    // Utility
    double getLoadPercentage() const;
};

#endif // SERVER_H

