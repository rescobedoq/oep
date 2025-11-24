#pragma once

#include <string>
#include <unordered_map>
#include <cstdint> 
#include "Node.h"
#include "../value_objects/Distance.h"

// Forward declaration
class VehicleProfile;

class Edge {
private:
    int64_t id;
    Node* source;
    Node* target;
    bool isOneWay;
    Distance distance;      // Distance in meters
    std::unordered_map<std::string, std::string> tags;

public:
    Edge(
        int64_t id,
        Node* source,
        Node* target,
        bool isOneWay = false,
        const Distance& distance = Distance(0),
        const std::unordered_map<std::string, std::string>& tags = {}
    );

    // Getters
    int64_t getId() const { return id; }
    Node* getSource() const { return source; }
    Node* getTarget() const { return target; }
    Distance& getDistance() { return distance; }
    bool IsOneWay() const { return isOneWay; }
    std::unordered_map<std::string, std::string>& getTags() { return tags; }
    const std::unordered_map<std::string, std::string>& getTags() const { return tags; }

    // Tag management
    void addTag(const std::string& key, const std::string& value);
    std::string getTag(const std::string& key) const;
    bool hasTag(const std::string& key) const;
    
    // Helper to get street name with fallback
    std::string getStreetName() const;

    // Comparation
    bool operator==(const Edge& other) const { return id == other.getId(); }
    bool operator!=(const Edge& other) const { return id != other.getId(); }
};