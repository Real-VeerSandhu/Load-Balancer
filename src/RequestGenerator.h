#ifndef REQUESTGENERATOR_H
#define REQUESTGENERATOR_H

class RequestGenerator {
public:
    // Generate a number of requests
    static int generate(int count);
    
    // Generate requests with a distribution (for future use)
    // Could be used for burst patterns, etc.
};

#endif // REQUESTGENERATOR_H

