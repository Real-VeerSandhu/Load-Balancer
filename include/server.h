// Structure to represent a server
struct Server {
    int id;
    double capacity;  // Maximum load capacity
    double currentLoad;
    double utilizationRatio() const {
        return (capacity > 0) ? currentLoad / capacity : 0;
    }
};
