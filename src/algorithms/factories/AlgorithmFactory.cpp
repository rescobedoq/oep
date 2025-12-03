#include "./AlgorithmFactory.h"
#include "../pathfinding/DijkstraAlgorithm.h"
#include "../pathfinding/AStarAlgorithm.h"
// #include "../pathfinding/ALTAlgorithm.h"

std::unique_ptr<IPathfindingAlgorithm> AlgorithmFactory::createAlgorithm(const std::string& algorithmName) {
    if (algorithmName == "dijkstra") {
        return std::make_unique<DijkstraAlgorithm>();
    } else if (algorithmName == "astar" || algorithmName == "a*" || algorithmName == "a_star") {
        return std::make_unique<AStarAlgorithm>();
    }
    // TODO: Implement ALT algorithm
    // else if (algorithmName == "alt") {
    //     return std::make_unique<ALTAlgorithm>();
    // } 
    else {
        throw std::invalid_argument("Unknown algorithm: " + algorithmName);
    }
}