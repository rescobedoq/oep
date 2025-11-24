// src/algorithms/tsp/TspMatrix.h
#pragma once

#include "../../core/entities/Graph.h"
#include "../../core/interfaces/IPathfindingAlgorithm.h"
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <memory>
#include <functional>

/**
 * @brief TSP Matrix with distances and precomputed paths
 * 
 * - parallel precompute with bidirectional cache
 * - nearestNeighborRoute() for heuristic initialization
 * - getEntry() to access distances/paths
 */
class TspMatrix {
public:
    /**
     * @brief Matrix entry (distance + path)
     */
    struct Entry {
        double distance;
        std::vector<int64_t> pathEdgeIds;
        
        Entry() : distance(0.0) {}
        Entry(double dist, const std::vector<int64_t>& path)
            : distance(dist), pathEdgeIds(path) {}
    };
    
    /**
     * @brief Progress callback for UI
     * 
     * @param current Completed pairs
     * @param total Total pairs
     * @param percent Percentage (0-100)
     */
    using ProgressCallback = std::function<void(int current, int total, int percent)>;
    
private:
    size_t size_;
    std::vector<int64_t> nodeIds_;
    
    // Matriz: matrix_[fromIdx][toIdx] = Entry
    std::vector<std::vector<Entry>> matrix_;
    
public:
    /**
     * @brief Constructor
     * 
     * @param size Number of waypoints
     * @param nodeIds Node IDs to visit
     */
    TspMatrix(size_t size, const std::vector<int64_t>& nodeIds);
    
    /**
     * @brief Precompute matrix using pathfinding
     * 
     * @param graph Graph
     * @param algorithm Pathfinding algorithm (Dijkstra, A*, etc.)
     * @param vehicleProfile Vehicle profile (can be nullptr)
     * @param progressCallback Callback for progress feedback (optional)
     */
    void precompute(
        const Graph& graph,
        IPathfindingAlgorithm* algorithm,
        const VehicleProfile* vehicleProfile = nullptr,
        ProgressCallback progressCallback = nullptr
    );
    
    /**
     * @brief Get matrix entry
     */
    const Entry& getEntry(size_t fromIdx, size_t toIdx) const {
        return matrix_[fromIdx][toIdx];
    }
    
    /**
     * @brief Get matrix entry by node IDs
     */
    const Entry& getEntry(int64_t fromNodeId, int64_t toNodeId) const;
    
    /**
     * @brief Get path of edges between two nodes
     */
    const std::vector<int64_t>& getPath(size_t fromIdx, size_t toIdx) const {
        return matrix_[fromIdx][toIdx].pathEdgeIds;
    }
    
    /**
     * @brief Manually set distance (for tests)
     */
    void setDistance(size_t fromIdx, size_t toIdx, double distance) {
        matrix_[fromIdx][toIdx].distance = distance;
    }
    
    /**
     * @brief Get node ID by index
     */
    int64_t getNodeId(size_t idx) const {
        return nodeIds_[idx];
    }
    
    /**
     * @brief Get index by node ID
     */
    size_t getNodeIndex(int64_t nodeId) const;
    
    /**
     * @brief Matrix size
     */
    size_t getSize() const {
        return size_;
    }
    
    /**
     * @brief Calculate tour cost
     * 
     * @param tour Tour indices (0 to N-1)
     * @param returnToStart Whether to return to the start
     * @return Total distance
     */
    double calculateTourCost(const std::vector<int>& tour, bool returnToStart = false) const;
    
    /**
     * @brief Nearest Neighbor heuristic initialization
     * 
     * MAPPING: TspMatrix.nearestNeighborRoute() in Java
     * 
     * @param startIdx Start index (0 to N-1)
     * @return Heuristic tour (indices)
     */
    std::vector<int> nearestNeighborRoute(int startIdx = 0) const;
    
    /**
     * @brief Validate that the matrix has no infinite distances
     * 
     * @return Vector of pairs (fromIdx, toIdx) with infinite distances
     */
    std::vector<std::pair<size_t, size_t>> getUnreachablePairs() const;
    
    /**
     * @brief Validate if the matrix has a valid solution
     * 
     * @return true if all nodes are reachable from each other
     */
    bool hasValidSolution() const;
    
private:
    /**
     * @brief Convert path of edges to total distance
     */
    double calculatePathDistance(const Graph& graph, const std::vector<int64_t>& edgeIds) const;
};
