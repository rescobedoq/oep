#include "TspAlgorithmFactory.h"
#include "../tsp/IGAlgorithm.h"
#include "../tsp/IGNAlgorithm.h"
#include "../tsp/ILSBAlgorithm.h"
// TODO: Implement these TSP algorithms
// #include "../tsp/IGSAAlgorithm.h"
#include <stdexcept>

std::unique_ptr<ITspAlgorithm> TspAlgorithmFactory::create(const std::string& algorithmName) {
    if (algorithmName == "ig" || algorithmName == "IG") {
        return std::make_unique<IGAlgorithm>();
    } else if (algorithmName == "ign" || algorithmName == "IGN") {
        return std::make_unique<IGNAlgorithm>();
    } else if (algorithmName == "ilsb" || algorithmName == "ILSB" || algorithmName == "ils_b" || algorithmName == "ILS_B") {
        return std::make_unique<ILSBAlgorithm>();
    } else if (algorithmName == "igsa" || algorithmName == "IGSA") {
        // IGSA requires threading - not implemented yet
        throw std::invalid_argument("IGSA algorithm requires threading implementation (not available yet)");
    } else {
        throw std::invalid_argument("Unknown TSP algorithm: " + algorithmName);
    }
}
