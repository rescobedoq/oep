#include "PathfindingService.h"
#include "../algorithms/factories/AlgorithmFactory.h"
#include "../utils/exceptions/GraphException.h"
#include <thread>
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
    
    // Calculate total distance
    double totalDistance = 0.0;
    for (int64_t edgeId : path) {
        Edge* edge = graph_->getEdge(edgeId);
        if (edge) {
            totalDistance += edge->getDistance().getMeters();
        }
    }
    
    PathResult result;
    result.pathEdgeIds = path;
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
    // Ejecutar en hilo separado
    std::thread([this, startId, endId, algorithmName, vehicleProfile]() {
        try {
            PathResult result = findPathSync(startId, endId, algorithmName, vehicleProfile);
            emit pathFound(result);
        } catch (const std::exception& e) {
            emit pathError(QString::fromStdString(e.what()));
        }
    }).detach();
}
