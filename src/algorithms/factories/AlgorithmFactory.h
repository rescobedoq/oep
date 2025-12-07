#include "../../core/interfaces/IPathfindingAlgorithm.h"
#include <memory>
#include <string>

class AlgorithmFactory {
public:
    static std::unique_ptr<IPathfindingAlgorithm> createAlgorithm(const std::string& algorithmName);
};