#pragma once

#include "../../core/interfaces/IPathfindingAlgorithm.h"
#include "../../core/entities/Graph.h"
#include "../VehicleProfile.h"
#include <cmath>

/**
 * @brief A* Algorithm for shortest path
 * 
 * Uses Euclidean distance heuristic based on geographic coordinates.
 * Formula: f(n) = g(n) + h(n)
 * - g(n) = actual cost from start to n
 * - h(n) = heuristic estimate from n to goal
 * - f(n) = estimated total cost of optimal path through n
 * 
 * References:
 * - Amit Patel (Stanford): https://theory.stanford.edu/~amitp/GameProgramming/
 * - Red Blob Games: https://www.redblobgames.com/pathfinding/a-star/
 */
class AStarAlgorithm : public IPathfindingAlgorithm {
private:
    size_t nodesExplored = 0;
    double executionTime = 0.0;
    
    // Heuristic scale factor to maintain admissibility
    static constexpr double HEURISTIC_SCALE = 0.95;
    
    // Max expansions to prevent infinite loops
    static constexpr int MAX_EXPANSIONS = 200000;
    
    struct QueueNode {
        int64_t nodeId;
        double fScore;  // f(n) = g(n) + h(n)
        
        bool operator>(const QueueNode& other) const {
            return fScore > other.fScore;
        }
    };
    
    /**
     * @brief Calculate Euclidean heuristic between two nodes
     * Uses Manhattan distance approximation for speed
     */
    double calculateHeuristic(const Graph& graph, int64_t fromId, int64_t toId) const;
    
    /**
     * @brief Check if edge is blocked for vehicle profile
     */
    bool isEdgeRestrictedForVehicle(const Edge& edge, const VehicleProfile* vehicleProfile) const;
    
public:
    AStarAlgorithm() : nodesExplored(0), executionTime(0.0) {}
    
    std::vector<int64_t> findPath(
        const Graph& graph, 
        int64_t startNodeId, 
        int64_t endNodeId
    ) override;
    
    std::vector<int64_t> findPath(
        const Graph& graph, 
        int64_t startNodeId, 
        int64_t endNodeId,
        const VehicleProfile* vehicleProfile
    ) override;
    
    std::string getName() const override { return "a_star"; }
    size_t getNodesExplored() const override { return nodesExplored; }
    double getExecutionTime() const override { return executionTime; }
};
