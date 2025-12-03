// src/algorithms/pathfinding/AStarAlgorithm.h
#pragma once

#include "../../core/interfaces/IPathfindingAlgorithm.h"
#include "../../core/entities/Graph.h"
#include "../../algorithms/VehicleProfile.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <cstdint>
#include <string>

/**
 * @brief A* pathfinding algorithm implementation
 * 
 * Uses euclidean distance heuristic based on geographic coordinates.
 * Based on references from Amit Patel (Stanford) and Red Blob Games.
 * Source: https://theory.stanford.edu/~amitp/GameProgramming/
 * Source: https://www.redblobgames.com/pathfinding/a-star/
 * 
 * A* Formula: f(n) = g(n) + h(n)
 * where:
 * - g(n) = actual cost from start to n
 * - h(n) = estimated heuristic from n to goal
 * - f(n) = estimated total cost of optimal path through n
 */
class AStarAlgorithm : public IPathfindingAlgorithm {
private:
    // Constants
    static constexpr double HEURISTIC_SCALE = 0.95;
    static constexpr int MAX_EXPANSIONS = 200000;
    
    // Metrics
    mutable size_t nodesExplored_;
    mutable double executionTime_;
    
    /**
     * @brief Node score for priority queue
     */
    struct NodeScore {
        int64_t nodeId;
        double fScore;
        
        NodeScore(int64_t id, double score) 
            : nodeId(id), fScore(score) {}
        
        bool operator>(const NodeScore& other) const {
            return fScore > other.fScore;
        }
    };

public:
    AStarAlgorithm();
    
    // IPathfindingAlgorithm interface implementation
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
    
    std::string getName() const override { return "A*"; }
    size_t getNodesExplored() const override { return nodesExplored_; }
    double getExecutionTime() const override { return executionTime_; }

private:
    /**
     * @brief Core A* calculation with optional vehicle profile
     */
    std::vector<int64_t> calculatePath(
        const Graph& graph,
        int64_t startNodeId,
        int64_t endNodeId,
        const VehicleProfile* vehicleProfile
    ) const;
    
    /**
     * @brief Calculate heuristic h(n) using euclidean distance
     * 
     * This is an admissible heuristic (never overestimates) for ground routes.
     * Uses Manhattan approximation scaled to meters for speed.
     */
    double calculateHeuristic(
        const Graph& graph,
        int64_t fromNodeId,
        int64_t toNodeId
    ) const;
    
    /**
     * @brief Check if edge is restricted for vehicle type
     */
    bool isEdgeRestrictedForVehicle(
        const Edge* edge,
        const VehicleProfile* profile
    ) const;
    
    /**
     * @brief Reconstruct path from edge map
     */
    std::vector<int64_t> reconstructPath(
        const std::unordered_map<int64_t, int64_t>& prevEdge,
        int64_t startNodeId,
        int64_t endNodeId
    ) const;
    
    /**
     * @brief Validate reconstructed path
     */
    void validatePath(
        const std::vector<int64_t>& path,
        const Graph& graph,
        size_t expansions
    ) const;
};