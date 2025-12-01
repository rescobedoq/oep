#pragma once

#include "../../core/interfaces/ITspAlgorithm.h"
#include "TspMatrix.h"
#include <random>

/**
 * @brief Iterated Greedy for TSP
 * 
 * - removeAndReinsert() → destruction + construction
 * - localSearch() → 2-opt swap
 * - 5000 iterations by default
 */
class IGAlgorithm : public ITspAlgorithm {
private:
    int maxIterations_;
    bool returnToStart_;
    
public:
    IGAlgorithm(int maxIterations = 5000, bool returnToStart = false)
        : maxIterations_(maxIterations)
        , returnToStart_(returnToStart)
    {}
    
    std::vector<int> solve(
        const TspMatrix& matrix,
        const std::vector<int64_t>& nodeIds
    ) override;
    
    std::string getName() const override {
        return "IG";
    }
    
    void setReturnToStart(bool value) {
        returnToStart_ = value;
    }
    
    void setMaxIterations(int maxIterations) override {
        maxIterations_ = maxIterations;
    }
    
private:
    /**
     * @brief Destruction + Construction
     */
    void removeAndReinsert(std::vector<int>& route, std::mt19937& rng);
    
    /**
     * @brief Local Search (2-opt swap)
     */
    double localSearch(std::vector<int>& route, const TspMatrix& matrix);
    
    /**
     * @brief Calculate route distance
     */
    double routeDistance(const std::vector<int>& route, const TspMatrix& matrix) const;
};
