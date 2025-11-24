#pragma once

#include <string>
#include <vector>
#include <cstdint>

class TspMatrix;

/*
 * - Polymorphism: Abstract main class with pure virtual methods
 * - Low coupling: Concrete algorithms depend only on this interface
*/

class ITspAlgorithm {
public:
    virtual ~ITspAlgorithm() = default;

    /**
     * @brief Resolve TSP on precalculated matrix
     * @param matrix TSP distance matrix
     * @param nodeIds IDs of nodes to visit
     * @return Tour as a vector of INDICES (0 to N-1)
     */
    virtual std::vector<int> solve(
        const TspMatrix& matrix,
        const std::vector<int64_t>& nodeIds
    ) = 0;

    virtual std::string getName() const = 0;

    virtual void setMaxIterations(int maxIterations) {}
    virtual void setTimeLimit(double seconds) {}
};