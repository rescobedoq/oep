#pragma once

#include <QObject>
#include <QFuture>
#include <memory>
#include <vector>
#include <cstdint>
#include "../core/entities/Graph.h"
#include "../core/interfaces/IPathfindingAlgorithm.h"
#include "../algorithms/VehicleProfile.h"

/**
 * @brief Service for calculating shortest paths
 * 
 * Supports sync and async operations
 */
class PathfindingService : public QObject {
    Q_OBJECT
    
public:
    /**
     * @brief Pathfinding result
     */
    struct PathResult {
        std::vector<Edge*> pathEdges;         // ✅ Punteros a edges (para nombres de calles)
        std::vector<int64_t> pathEdgeIds;     // IDs de edges (backward compatibility)
        std::vector<int64_t> pathNodeIds;     // IDs de nodos en orden
        double totalDistance;
        size_t nodesExplored;
        double executionTimeMs;
        std::string algorithmName;
        
        PathResult()
            : totalDistance(0.0)
            , nodesExplored(0)
            , executionTimeMs(0.0)
        {}
    };
    
private:
    std::shared_ptr<Graph> graph_;
    QFuture<void> pathFuture_;
    
public:
    explicit PathfindingService(QObject* parent = nullptr);
    
    /**
     * @brief Sets the graph to use
     */
    void setGraph(std::shared_ptr<Graph> graph) {
        graph_ = graph;
    }
    
    /**
     * @brief Calculates shortest path (SYNC - may freeze UI if heavy)
     */
    PathResult findPathSync(
        int64_t startId,
        int64_t endId,
        const std::string& algorithmName,
        const VehicleProfile* vehicleProfile = nullptr
    );
    
    /**
     * @brief Calculates shortest path (ASYNC - does NOT freeze UI)
     * 
     * Runs in std::thread and emits pathFound() signal when done
     */
    void findPathAsync(
        int64_t startId,
        int64_t endId,
        const std::string& algorithmName,
        const VehicleProfile* vehicleProfile = nullptr
    );
    
signals:
    void pathFound(PathResult result);
    void pathError(QString errorMessage);
};
