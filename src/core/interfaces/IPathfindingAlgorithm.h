#pragma once

#include <string>
#include <vector>
#include <cstdint>

class Graph;
class VehicleProfile;

/*
 * - Polymorphism: Abstract main class with pure virtual methods
 * - Low coupling: Concrete algorithms depend only on this interface
*/

class IPathfindingAlgorithm {
public:
    virtual ~IPathfindingAlgorithm() = default;

    // Search for a path in the graph from startNodeId to endNodeId
    virtual std::vector<int64_t> findPath(
        const Graph& graph,
        int64_t startNodeId,
        int64_t endNodeId
    ) = 0;

    // Search for a path in the graph from startNodeId to endNodeId WITH VEHICLE RESTRICTIONS
    virtual std::vector<int64_t> findPath(
        const Graph& graph,
        int64_t startNodeId,
        int64_t endNodeId,
        const VehicleProfile* vehicleProfile
    ) = 0;
    
    // Name of the algorithm (for logging/debugging)
    virtual std::string getName() const = 0;

    virtual size_t getNodesExplored() const = 0;
    virtual double getExecutionTime() const = 0;
};