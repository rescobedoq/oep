#include "IGAlgorithm.h"
#include <iostream>
#include <algorithm>

std::vector<int> IGAlgorithm::solve(
    const TspMatrix& matrix,
    const std::vector<int64_t>& nodeIds
) {
    // Heuristic initialization: Nearest Neighbor
    std::vector<int> initialRoute = matrix.nearestNeighborRoute(0);
    
    std::vector<int> best = initialRoute;
    double bestDist = routeDistance(best, matrix);
    
    std::vector<int> current = best;
    bestDist = localSearch(current, matrix);
    best = current;
    
    std::random_device rd;
    std::mt19937 rng(rd());
    
    // Iterated Greedy: 5000 iterations by default
    for (int iter = 0; iter < maxIterations_; iter++) {
        std::vector<int> temp = current;
        
        // Destruction + Construction
        removeAndReinsert(temp, rng);
        
        // Local Search
        double dist = localSearch(temp, matrix);
        
        // Aceptation
        if (dist < bestDist) {
            bestDist = dist;
            best = temp;
            current = temp;
        } else {
            current = best; // Reset to best solution
        }
        
        // Log every 1000 iterations
        if (iter % 1000 == 0) {
            std::cout << "[IG] Iteration " << iter << "/" << maxIterations_ 
                      << " | Best: " << bestDist << " m" << std::endl;
        }
    }
    
    std::cout << "[IGAlgorithm] Optimal distance: " << bestDist << " m" << std::endl;
    
    return best;
}

void IGAlgorithm::removeAndReinsert(std::vector<int>& route, std::mt19937& rng) {
    if (route.size() < 4) {
        return; // Not enough nodes to destroy
    }
    
    int n = std::min(3, static_cast<int>(route.size()));
    std::vector<int> removed;
    
    // Destruction: Remove n random nodes
    for (int k = 0; k < n; k++) {
        std::uniform_int_distribution<int> dist(0, route.size() - 1);
        int idx = dist(rng);
        removed.push_back(route[idx]);
        route.erase(route.begin() + idx);
    }
    
    // Construction: Reinsert nodes at random positions
    for (int node : removed) {
        std::uniform_int_distribution<int> dist(0, route.size());
        int idx = dist(rng);
        route.insert(route.begin() + idx, node);
    }
}

double IGAlgorithm::localSearch(std::vector<int>& route, const TspMatrix& matrix) {
    bool improved = true;
    double bestDist = routeDistance(route, matrix);
    
    while (improved) {
        improved = false;
        
        // 2-opt swap
        for (size_t i = 0; i < route.size(); i++) {
            for (size_t j = i + 1; j < route.size(); j++) {
                std::swap(route[i], route[j]);
                
                double dist = routeDistance(route, matrix);
                
                if (dist < bestDist) {
                    bestDist = dist;
                    improved = true;
                } else {
                    std::swap(route[i], route[j]); // Revert
                }
            }
        }
    }
    
    return bestDist;
}

double IGAlgorithm::routeDistance(const std::vector<int>& route, const TspMatrix& matrix) const {
    return matrix.calculateTourCost(route, returnToStart_);
}