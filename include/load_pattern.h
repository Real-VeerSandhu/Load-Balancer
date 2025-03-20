// load_pattern.h
#ifndef LOAD_PATTERN_H
#define LOAD_PATTERN_H

#include <vector>
#include <random>
#include <string>
#include <functional>
#include <chrono>
#include <memory>

enum class PatternType {
    CONSTANT,
    RANDOM,
    SINE_WAVE,
    SPIKE,
    GRADUAL_INCREASE,
    GRADUAL_DECREASE,
    DIURNAL,       // Day/night pattern
    WEEKLY,        // Weekly pattern with weekend dips
    BURSTY         // Random bursts
};

class LoadPatternGenerator {
private:
    std::mt19937 rng;
    PatternType currentPattern;
    double baseLoadLevel;
    double amplitudeFactor;
    double frequencyFactor;
    
    // For time-based patterns
    std::chrono::time_point<std::chrono::system_clock> startTime;
    std::chrono::time_point<std::chrono::system_clock> lastUpdateTime;
    double elapsedSeconds;
    
    // For complex patterns
    struct PatternState {
        virtual ~PatternState() = default;
        virtual double getNextLoad(LoadPatternGenerator* generator) = 0;
    };
    
    std::unique_ptr<PatternState> patternState;
    
    // Callback for when loads are generated
    std::function<void(int)> loadGeneratedCallback;
    
public:
    LoadPatternGenerator();
    
    // Pattern selection and configuration
    void setPattern(PatternType pattern);
    void setBaseLoadLevel(double baseLoad);
    void setAmplitude(double amplitude);
    void setFrequency(double frequency);
    
    // Generate loads
    int generateNextLoad();
    std::vector<int> generateBatchLoad(int count);
    
    // Scheduled generation
    void startScheduledGeneration(int intervalMs, int duration = 0);
    void stopScheduledGeneration();
    
    // Pattern-specific methods
    void configureDiurnalPattern(int peakHour = 14, int lowHour = 3);
    void configureWeeklyPattern(double weekendLoadFactor = 0.5);
    void configureBurstyPattern(double burstProbability = 0.1, double burstMultiplier = 5.0);
    
    // Callback
    void setLoadGeneratedCallback(std::function<void(int)> callback);
    
    // Utility
    static std::string patternTypeToString(PatternType type);
    PatternType getCurrentPattern() const;
    double getBaseLoadLevel() const;
};

#endif // LOAD_PATTERN_H