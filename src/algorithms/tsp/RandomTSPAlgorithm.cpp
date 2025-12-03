// src/algorithms/tsp/RandomTSPAlgorithm.cpp
#include "RandomTSPAlgorithm.h"
#include <iostream>
#include <algorithm>
#include <limits>

RandomTSPAlgorithm::RandomTSPAlgorithm(int maxIterations)
    : maxIterations_(maxIterations)
    , rng_(std::random_device{}()) {}

std::vector<int> RandomTSPAlgorithm::solve(
    const TspMatrix& matrix,
    const std::vector<int64_t>& nodeIds
) {
    if (nodeIds.empty()) {
        return {};
    }
    
    // Initialize with sequential indices [0, 1, 2, ..., N-1]
    std::vector<int> bestRoute;
    for (size_t i = 0; i < matrix.getSize(); ++i) {
        bestRoute.push_back(static_cast<int>(i));
    }
    
    double bestDistance = std::numeric_limits<double>::max();
    std::vector<int> tempRoute = bestRoute;
    
    // Random search iterations
    for (int iteration = 0; iteration < maxIterations_; ++iteration) {
        // Shuffle route randomly
        std::shuffle(tempRoute.begin(), tempRoute.end(), rng_);
        
        // Evaluate distance
        double distance = calculateTourDistance(matrix, tempRoute, false);
        
        // Keep best solution
        if (distance < bestDistance) {
            bestDistance = distance;
            bestRoute = tempRoute;
        }
    }
    
    // Ensure route starts with node 0
    bestRoute = ensureStartNode(bestRoute, 0);
    
    // Final validation
    double finalDistance = calculateTourDistance(matrix, bestRoute, false);
    
    std::cout << "[RandomTSPAlgorithm] Optimal route: ";
    for (int idx : bestRoute) {
        std::cout << idx << " ";
    }
    std::cout << std::endl;
    std::cout << "[RandomTSPAlgorithm] Optimal distance: " << finalDistance << std::endl;
    
    return bestRoute;
}

std::vector<int> RandomTSPAlgorithm::ensureStartNode(
    const std::vector<int>& route,
    int startIdx
) const {
    if (route.empty() || route[0] == startIdx) {
        return route;
    }
    
    // Find position of start node
    auto it = std::find(route.begin(), route.end(), startIdx);
    if (it == route.end()) {
        return route;  // Start node not found, return as is
    }
    
    // Rotate route to start with startIdx
    std::vector<int> rotatedRoute;
    rotatedRoute.insert(rotatedRoute.end(), it, route.end());
    rotatedRoute.insert(rotatedRoute.end(), route.begin(), it);
    
    return rotatedRoute;
}

double RandomTSPAlgorithm::calculateTourDistance(
    const TspMatrix& matrix,
    const std::vector<int>& route,
    bool returnToStart
) const {
    if (route.size() < 2) {
        return 0.0;
    }
    
    double totalDistance = 0.0;
    
    // Sum distances between consecutive nodes
    for (size_t i = 0; i < route.size() - 1; ++i) {
        totalDistance += matrix.getEntry(static_cast<size_t>(route[i]), static_cast<size_t>(route[i + 1])).distance;
    }
    
    // Add return to start if needed
    if (returnToStart && route.size() > 1) {
        totalDistance += matrix.getEntry(static_cast<size_t>(route.back()), static_cast<size_t>(route[0])).distance;
    }
    
    return totalDistance;
}