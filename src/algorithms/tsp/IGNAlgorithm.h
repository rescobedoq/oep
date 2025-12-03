// src/algorithms/tsp/IGNAlgorithm.h
#pragma once

#include "../../core/interfaces/ITspAlgorithm.h"
#include "TspMatrix.h"
#include <vector>
#include <random>
#include <cstdint>

/**
 * @brief Iterated Greedy with Noise (IGN) Algorithm for TSP
 * 
 * Metaheuristic that iteratively removes and reinserts nodes
 * to explore the solution space. Uses greedy nearest neighbor
 * as initial solution.
 * 
 * Algorithm steps:
 * 1. Start with nearest neighbor heuristic
 * 2. Remove random nodes (destruction phase)
 * 3. Reinsert nodes at random positions (construction phase)
 * 4. Accept if better solution found
 * 5. Repeat for max iterations
 */
class IGNAlgorithm : public ITspAlgorithm {
private:
    int maxIterations_;
    mutable std::mt19937 rng_;
    
public:
    explicit IGNAlgorithm(int maxIterations = 10000);
    
    // ITspAlgorithm interface implementation
    std::vector<int> solve(
        const TspMatrix& matrix,
        const std::vector<int64_t>& nodeIds
    ) override;
    
    std::string getName() const override { return "IGN"; }
    
    void setMaxIterations(int maxIterations) override {
        maxIterations_ = maxIterations;
    }

private:
    /**
     * @brief Ensure tour starts with the first node in nodeIds
     */
    std::vector<int> ensureStartNode(
        const std::vector<int>& route,
        int startIdx
    ) const;
    
    /**
     * @brief Remove and reinsert nodes (destruction + construction)
     */
    void removeAndReinsert(
        std::vector<int>& route,
        int numNodesToRemove
    ) const;
    
    /**
     * @brief Calculate total tour distance
     */
    double calculateTourDistance(
        const TspMatrix& matrix,
        const std::vector<int>& route,
        bool returnToStart
    ) const;
};