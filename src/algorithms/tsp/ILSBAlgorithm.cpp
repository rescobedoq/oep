#include "ILSBAlgorithm.h"
#include <algorithm>
#include <iostream>
#include <limits>

double ILSBAlgorithm::localSearch(std::vector<int>& route, const TspMatrix& matrix) {
    bool improved = true;
    double bestDist = routeDistance(route, matrix);
    
    while (improved) {
        improved = false;
        
        // Try all 2-opt moves (reverse segments, not simple swaps)
        for (size_t i = 0; i < route.size() - 1; i++) {
            for (size_t j = i + 2; j < route.size(); j++) {
                // Reverse segment [i+1, j]
                std::reverse(route.begin() + i + 1, route.begin() + j + 1);
                
                double dist = routeDistance(route, matrix);
                
                if (dist < bestDist) {
                    bestDist = dist;
                    improved = true;
                } else {
                    // Revert: reverse again to undo
                    std::reverse(route.begin() + i + 1, route.begin() + j + 1);
                }
            }
        }
    }
    
    return bestDist;
}

double ILSBAlgorithm::routeDistance(const std::vector<int>& route, const TspMatrix& matrix) const {
    return matrix.calculateTourCost(route, returnToStart_);
}

void ILSBAlgorithm::ensureStartNode(std::vector<int>& route, int startIndex) {
    if (route.empty() || route[0] == startIndex) return;
    
    auto it = std::find(route.begin(), route.end(), startIndex);
    if (it == route.end()) return;
    
    // Rotate to start with startIndex
    std::rotate(route.begin(), it, route.end());
}

std::vector<int> ILSBAlgorithm::solve(
    const TspMatrix& matrix,
    const std::vector<int64_t>& nodeIds
) {
    if (nodeIds.empty()) {
        return {};
    }
    
    // Get initial solution using Nearest Neighbor
    std::vector<int> currentRoute = matrix.nearestNeighborRoute(0);  // Start from index 0
    std::vector<int> bestRoute = currentRoute;
    
    // Apply initial local search
    double bestDist = localSearch(currentRoute, matrix);
    bestRoute = currentRoute;
    
    std::random_device rd;
    std::mt19937 rng(rd());
    
    std::cout << "[ILSB] Initial distance after local search: " << bestDist << std::endl;
    
    // Iterated Local Search iterations
    for (int iter = 0; iter < maxIterations_; iter++) {
        // Perturbation: shuffle a copy of best route
        currentRoute = bestRoute;
        std::shuffle(currentRoute.begin(), currentRoute.end(), rng);
        
        // Local search
        double dist = localSearch(currentRoute, matrix);
        
        // Accept if better
        if (dist < bestDist) {
            bestDist = dist;
            bestRoute = currentRoute;
        }
    }
    
    // Ensure route starts with first node
    ensureStartNode(bestRoute, 0);
    
    double finalDist = routeDistance(bestRoute, matrix);
    std::cout << "[ILSB] Optimal route distance: " << finalDist << std::endl;
    std::cout << "[ILSB] Route size: " << bestRoute.size() << " nodes" << std::endl;
    
    return bestRoute;
}
