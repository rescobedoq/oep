#pragma once

#include <QObject>
#include <memory>
#include <vector>
#include <cstdint>
#include "../core/entities/Graph.h"
#include "../algorithms/VehicleProfile.h"

/**
 * @brief Service for solving TSP
 * 
 * Includes:
 * - TSP matrix precompute (parallel in Java, sequential here)
 * - Solve with TSP algorithm (IG, IGSA, etc.)
 * - Async execution to NOT freeze UI
 */
class TspService : public QObject {
    Q_OBJECT
    
public:
    /**
     * @brief TSP result
     */
    struct TspResult {
        std::vector<int> tour;           // Tour indices
        std::vector<int64_t> nodeIds;    // Original node IDs
        double totalDistance;
        double executionTimeMs;
        double precomputeTimeMs;
        std::string tspAlgorithmName;
        
        TspResult()
            : totalDistance(0.0)
            , executionTimeMs(0.0)
            , precomputeTimeMs(0.0)
        {}
    };
    
private:
    std::shared_ptr<Graph> graph_;
    
public:
    explicit TspService(QObject* parent = nullptr);
    
    void setGraph(std::shared_ptr<Graph> graph) {
        graph_ = graph;
    }
    
    /**
     * @brief Solves TSP (ASYNC - does NOT freeze UI)
     * 
     * Steps:
     * 1. Create TspMatrix
     * 2. Precompute (emits progress signals)
     * 3. Solve with TSP algorithm
     * 4. Emit tspSolved()
     */
    void solveAsync(
        const std::vector<int64_t>& waypointIds,
        const std::string& tspAlgorithmName,
        const std::string& pathfindingAlgorithmName = "dijkstra",
        const VehicleProfile* vehicleProfile = nullptr,
        bool returnToStart = false
    );
    
signals:
    void precomputeProgress(int percent);
    void tspSolved(TspResult result);
    void tspError(QString errorMessage);
};