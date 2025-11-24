#include "VehicleProfile.h"
#include <algorithm>

VehicleProfile::VehicleProfile(const std::string& name, const std::string& type)
    : name(name), type(type) {}

void VehicleProfile::setSpeedFactor(const std::string& roadType, double factor) {
    speedFactors[roadType] = factor;
}

void VehicleProfile::setSpeed(double speed) {
    this->speed = speed;
}

double VehicleProfile::getSpeedFactor(const std::string& roadType) const {
    auto it = speedFactors.find(roadType);
    return (it != speedFactors.end()) ? it->second : 1.0;
}

bool VehicleProfile::isHighwayBlocked(const std::string& highwayType) const {
    double factor = getSpeedFactor(highwayType);
    return factor <= 0.0;
}

bool VehicleProfile::isRoadSuitable(const std::unordered_map<std::string, std::string>& tags) const {
    auto it = tags.find("highway");
    if (it == tags.end()) return true; // no info = allowed

    const std::string& highway = it->second;
    return !isHighwayBlocked(highway);
}

std::vector<std::string> VehicleProfile::getPreferredTags() const {
    std::vector<std::string> result;
    for (const auto& [tag, factor] : speedFactors)
        if (factor > 1.0) result.push_back(tag);
    return result;
}

std::vector<std::string> VehicleProfile::getAvoidedTags() const {
    std::vector<std::string> result;
    for (const auto& [tag, factor] : speedFactors)
        if (factor < 1.0) result.push_back(tag);
    return result;
}

std::vector<std::string> VehicleProfile::getBlockedTags() const {
    std::vector<std::string> blocked;
    for (const auto& [tag, factor] : speedFactors)
        if (factor <= 0.0) blocked.push_back(tag);
    return blocked;
}