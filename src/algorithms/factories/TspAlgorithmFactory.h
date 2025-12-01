#pragma once

#include "../../core/interfaces/ITspAlgorithm.h"
#include <memory>
#include <string>

class TspAlgorithmFactory {
public:
    /**
     * @brief Create a TSP algorithm by name
     * 
     * Supported algorithms:
     * - "ig" → IGAlgorithm
     * - "igsa" → IGSAAlgorithm
     * - "ilsa" → ILSAAlgorithm
     * - ...
     */
    static std::unique_ptr<ITspAlgorithm> create(const std::string& algorithmName);
};
