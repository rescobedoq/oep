#include "TspAlgorithmFactory.h"
#include "../tsp/IGAlgorithm.h"
// TODO: Implement these TSP algorithms
// #include "../tsp/IGSAAlgorithm.h"
// #include "../tsp/ILSAAlgorithm.h"
// ...
#include <stdexcept>

std::unique_ptr<ITspAlgorithm> TspAlgorithmFactory::create(const std::string& algorithmName) {
    if (algorithmName == "ig" || algorithmName == "IG") {
        return std::make_unique<IGAlgorithm>();
    } 
    // TODO: Implement these algorithms
    // else if (algorithmName == "igsa" || algorithmName == "IGSA") {
    //     return std::make_unique<IGSAAlgorithm>();
    // } else if (algorithmName == "ilsa" || algorithmName == "ILSA") {
    //     return std::make_unique<ILSAAlgorithm>();
    // } else if ...
    else {
        throw std::invalid_argument("Unknown TSP algorithm: " + algorithmName);
    }
}
