#include "IGNAlgorithm.h"
#include <algorithm>
#include <iostream>
#include <limits>

void IGNAlgorithm::removeAndReinsert(std::vector<int>& route, std::mt19937& rng) {
    if (route.size() < 3) return;
    
    int n = std::min(3, static_cast<int>(route.size()));
    std::vector<int> removed;
    
    // Remove n random nodes
    for (int k = 0; k < n; k++) {
        std::uniform_int_distribution<int> dist(0, route.size() - 1);
        int idx = dist(rng);
        removed.push_back(route[idx]);
        route.erase(route.begin() + idx);
    }
    
    // Reinsert at random positions
    for (int node : removed) {
        std::uniform_int_distribution<int> dist(0, route.size());
        int idx = dist(rng);
        route.insert(route.begin() + idx, node);
    }
}

double IGNAlgorithm::routeDistance(const std::vector<int>& route, const TspMatrix& matrix) const {
    return matrix.calculateTourCost(route, returnToStart_);
}

void IGNAlgorithm::ensureStartNode(std::vector<int>& route, int startIndex) {
    if (route.empty() || route[0] == startIndex) return;
    
    auto it = std::find(route.begin(), route.end(), startIndex);
    if (it == route.end()) return;
    
    // Rotate to start with startIndex
    std::rotate(route.begin(), it, route.end());
}

std::vector<int> IGNAlgorithm::solve(
    const TspMatrix& matrix,
    const std::vector<int64_t>& nodeIds
) {
    if (nodeIds.empty()) {
        return {};
    }
    
    // Get initial solution using Nearest Neighbor
    std::vector<int> currentRoute = matrix.nearestNeighborRoute(0);  // Start from index 0
    std::vector<int> bestRoute = currentRoute;
    double bestDist = routeDistance(bestRoute, matrix);
    
    std::random_device rd;
    std::mt19937 rng(rd());
    
    std::cout << "[IGN] Initial distance: " << bestDist << std::endl;
    
    // Iterated Greedy iterations
    for (int iter = 0; iter < maxIterations_; iter++) {
        std::vector<int> tempRoute = currentRoute;
        
        // Destruction + Construction
        removeAndReinsert(tempRoute, rng);
        
        double dist = routeDistance(tempRoute, matrix);
        
        // Accept if better
        if (dist < bestDist) {
            bestDist = dist;
            bestRoute = tempRoute;
            currentRoute = tempRoute;
        } else {
            // Reset to best (greedy acceptance)
            currentRoute = bestRoute;
        }
    }
    
    // Ensure route starts with first node
    ensureStartNode(bestRoute, 0);
    
    double finalDist = routeDistance(bestRoute, matrix);
    std::cout << "[IGN] Optimal route distance: " << finalDist << std::endl;
    std::cout << "[IGN] Route size: " << bestRoute.size() << " nodes" << std::endl;
    
    return bestRoute;
}
