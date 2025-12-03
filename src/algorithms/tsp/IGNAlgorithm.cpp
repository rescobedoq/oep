// src/algorithms/tsp/IGNAlgorithm.cpp
#include "IGNAlgorithm.h"
#include <iostream>
#include <algorithm>

IGNAlgorithm::IGNAlgorithm(int maxIterations)
    : maxIterations_(maxIterations)
    , rng_(std::random_device{}()) {}

std::vector<int> IGNAlgorithm::solve(
    const TspMatrix& matrix,
    const std::vector<int64_t>& nodeIds
) {
    if (nodeIds.empty()) {
        return {};
    }
    
    // Determine start index from provided nodeIds (if any)
    int startIdx = 0;
    if (!nodeIds.empty()) {
        size_t candidate = matrix.getNodeIndex(nodeIds[0]);
        if (candidate < matrix.getSize()) startIdx = static_cast<int>(candidate);
    }

    // Get initial solution using nearest neighbor heuristic
    std::vector<int> initialRoute = matrix.nearestNeighborRoute(startIdx);
    
    std::vector<int> bestRoute = initialRoute;
    double bestDistance = calculateTourDistance(matrix, bestRoute, false);
    
    std::vector<int> currentRoute = bestRoute;
    
    // Iterated Greedy iterations
    for (int iteration = 0; iteration < maxIterations_; ++iteration) {
        std::vector<int> tempRoute = currentRoute;
        
        // Destruction + Construction phase
        int numToRemove = std::min(3, static_cast<int>(tempRoute.size()));
        removeAndReinsert(tempRoute, numToRemove);

        // Evaluate new solution using TspMatrix helper
        double distance = matrix.calculateTourCost(tempRoute, false);
        
        // Acceptance criterion (greedy)
        if (distance < bestDistance) {
            bestDistance = distance;
            bestRoute = tempRoute;
            currentRoute = tempRoute;
        } else {
            // Restart from best
            currentRoute = bestRoute;
        }
    }
    
    // Ensure route starts with the requested start index
    bestRoute = ensureStartNode(bestRoute, startIdx);

    // Final validation (use matrix helper)
    double finalDistance = matrix.calculateTourCost(bestRoute, false);

    (void)finalDistance; // silence unused variable when logging is disabled
    
    return bestRoute;
}

std::vector<int> IGNAlgorithm::ensureStartNode(
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

void IGNAlgorithm::removeAndReinsert(
    std::vector<int>& route,
    int numNodesToRemove
) const {
    if (route.size() <= 1) return;
    
    std::vector<int> removedNodes;
    
    // Destruction phase: remove random nodes
    for (int k = 0; k < numNodesToRemove && !route.empty(); ++k) {
        std::uniform_int_distribution<int> dist(0, static_cast<int>(route.size()) - 1);
        int idx = dist(rng_);
        removedNodes.push_back(route[idx]);
        route.erase(route.begin() + idx);
    }
    
    // Construction phase: reinsert at random positions
    for (int node : removedNodes) {
        if (route.empty()) {
            route.push_back(node);
        } else {
            std::uniform_int_distribution<int> insertDist(0, static_cast<int>(route.size()));
            int idx = insertDist(rng_);
            route.insert(route.begin() + idx, node);
        }
    }
}

double IGNAlgorithm::calculateTourDistance(
    const TspMatrix& matrix,
    const std::vector<int>& route,
    bool returnToStart
) const {
    // Delegate to TspMatrix implementation (keeps logic centralized)
    return matrix.calculateTourCost(route, returnToStart);
}