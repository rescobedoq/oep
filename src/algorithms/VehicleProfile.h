#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <stdexcept>
#include <cmath>

class VehicleProfile {
private:
    std::string name;
    std::string type;
    double speed;      // km/h
    std::unordered_map<std::string, double> speedFactors;

public:
    VehicleProfile(const std::string& name, const std::string& type);

    // Setters
    void setSpeedFactor(const std::string& roadType, double factor);
    void setSpeed(double speed);

    // Getters
    const std::string& getName() const { return name; }
    const std::string& getType() const { return type; }
    double getSpeed() const { return speed; }
    std::unordered_map<std::string, double> getSpeedFactors() const { return speedFactors; }
    double getSpeedFactor(const std::string& roadType) const;

    // Calculate effective speed and suitability
    bool isRoadSuitable(const std::unordered_map<std::string, std::string>& tags) const;
    bool isHighwayBlocked(const std::string& highwayType) const;

    // Get preferred tags and avoided tags
    std::vector<std::string> getPreferredTags() const;
    std::vector<std::string> getAvoidedTags() const;
    std::vector<std::string> getBlockedTags() const;
};
