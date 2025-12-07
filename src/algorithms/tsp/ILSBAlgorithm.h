#pragma once

#include "../../core/interfaces/ITspAlgorithm.h"
#include "TspMatrix.h"
#include <random>

/**
 * @brief Iterated Local Search Basic for TSP
 * 
 * - Uses shuffle for perturbation
 * - Local search with 2-opt swaps
 * - 5000 iterations by default
 * - Simpler than IG, faster convergence
 */
class ILSBAlgorithm : public ITspAlgorithm {
private:
    int maxIterations_;
    bool returnToStart_;
    
public:
    ILSBAlgorithm(int maxIterations = 5000, bool returnToStart = false)
        : maxIterations_(maxIterations)
        , returnToStart_(returnToStart)
    {}
    
    std::vector<int> solve(
        const TspMatrix& matrix,
        const std::vector<int64_t>& nodeIds
    ) override;
    
    std::string getName() const override {
        return "ILSB";
    }
    
    void setReturnToStart(bool value) {
        returnToStart_ = value;
    }
    
    void setMaxIterations(int maxIterations) override {
        maxIterations_ = maxIterations;
    }
    
private:
    /**
     * @brief Local search using 2-opt swaps
     * Returns the best distance found
     */
    double localSearch(std::vector<int>& route, const TspMatrix& matrix);
    
    /**
     * @brief Calculate total route distance
     */
    double routeDistance(const std::vector<int>& route, const TspMatrix& matrix) const;
    
    /**
     * @brief Ensure route starts with the first node
     */
    void ensureStartNode(std::vector<int>& route, int startIndex);
};
