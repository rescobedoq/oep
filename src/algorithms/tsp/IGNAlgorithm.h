#pragma once

#include "../../core/interfaces/ITspAlgorithm.h"
#include "TspMatrix.h"
#include <random>

/**
 * @brief Iterated Greedy with Nearest Neighbor for TSP
 * 
 * Similar to IG but uses Nearest Neighbor for initial solution
 * - removeAndReinsert() → destruction + construction (3 nodes)
 * - No local search (faster than IG)
 * - 10000 iterations by default
 */
class IGNAlgorithm : public ITspAlgorithm {
private:
    int maxIterations_;
    bool returnToStart_;
    
public:
    IGNAlgorithm(int maxIterations = 10000, bool returnToStart = false)
        : maxIterations_(maxIterations)
        , returnToStart_(returnToStart)
    {}
    
    std::vector<int> solve(
        const TspMatrix& matrix,
        const std::vector<int64_t>& nodeIds
    ) override;
    
    std::string getName() const override {
        return "IGN";
    }
    
    void setReturnToStart(bool value) {
        returnToStart_ = value;
    }
    
    void setMaxIterations(int maxIterations) override {
        maxIterations_ = maxIterations;
    }
    
private:
    /**
     * @brief Remove and reinsert 3 random nodes
     */
    void removeAndReinsert(std::vector<int>& route, std::mt19937& rng);
    
    /**
     * @brief Calculate total route distance
     */
    double routeDistance(const std::vector<int>& route, const TspMatrix& matrix) const;
    
    /**
     * @brief Ensure route starts with the first node
     */
    void ensureStartNode(std::vector<int>& route, int startIndex);
};
