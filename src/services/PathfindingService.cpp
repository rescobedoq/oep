#include "PathfindingService.h"
#include "../algorithms/factories/AlgorithmFactory.h"
#include "../utils/exceptions/GraphException.h"
#include <QtConcurrent/QtConcurrent>
#include <chrono>

PathfindingService::PathfindingService(QObject* parent)
    : QObject(parent)
    , graph_(nullptr)
{}

PathfindingService::PathResult PathfindingService::findPathSync(
    int64_t startId,
    int64_t endId,
    const std::string& algorithmName,
    const VehicleProfile* vehicleProfile
) {
    if (!graph_) {
        throw GraphException("Graph not loaded");
    }
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Create algorithm
    auto algorithm = AlgorithmFactory::createAlgorithm(algorithmName);
    
    // Execute pathfinding
    std::vector<int64_t> path;
    if (vehicleProfile) {
        path = algorithm->findPath(*graph_, startId, endId, vehicleProfile);
    } else {
        path = algorithm->findPath(*graph_, startId, endId);
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    double executionTimeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();
    
    // Calculate total distance and collect Edge pointers + Node IDs
    double totalDistance = 0.0;
    std::vector<Edge*> pathEdges;
    std::vector<int64_t> pathNodeIds;
    
    // Add start node
    if (!path.empty()) {
        Edge* firstEdge = graph_->getEdge(path[0]);
        if (firstEdge && firstEdge->getSource()) {
            pathNodeIds.push_back(firstEdge->getSource()->getId());
        }
    }
    
    for (int64_t edgeId : path) {
        Edge* edge = graph_->getEdge(edgeId);
        if (edge) {
            pathEdges.push_back(edge);
            totalDistance += edge->getDistance().getMeters();
            
            // Add target node
            if (edge->getTarget()) {
                pathNodeIds.push_back(edge->getTarget()->getId());
            }
        }
    }
    
    PathResult result;
    result.pathEdges = pathEdges;        // Edge pointers
    result.pathEdgeIds = path;           // Ids (backward compatibility)
    result.pathNodeIds = pathNodeIds;    // Node Ids en orden
    result.totalDistance = totalDistance;
    result.nodesExplored = algorithm->getNodesExplored();
    result.executionTimeMs = executionTimeMs;
    result.algorithmName = algorithmName;
    
    return result;
}

void PathfindingService::findPathAsync(
    int64_t startId,
    int64_t endId,
    const std::string& algorithmName,
    const VehicleProfile* vehicleProfile
) {
    // Copiar VehicleProfile si existe (para evitar use-after-free en thread asíncrono)
    std::unique_ptr<VehicleProfile> vehicleProfileCopy = nullptr;
    if (vehicleProfile) {
        vehicleProfileCopy = std::make_unique<VehicleProfile>(*vehicleProfile);
    }
    
    // Execute in Qt thread pool (thread-safe with Qt signals)
    pathFuture_ = QtConcurrent::run([this, startId, endId, algorithmName, 
                                     vehicleProfileCopy = std::move(vehicleProfileCopy)]() {
        try {
            PathResult result = findPathSync(startId, endId, algorithmName, vehicleProfileCopy.get());
            emit pathFound(result);
        } catch (const std::exception& e) {
            emit pathError(QString::fromStdString(e.what()));
        }
    });
}
