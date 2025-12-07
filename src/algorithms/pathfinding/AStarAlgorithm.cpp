#include "AStarAlgorithm.h"
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <chrono>

double AStarAlgorithm::calculateHeuristic(const Graph& graph, int64_t fromId, int64_t toId) const {
    const Node* fromNode = graph.getNode(fromId);
    const Node* toNode = graph.getNode(toId);
    
    if (!fromNode || !toNode) {
        return 0.0;
    }
    
    // Manhattan distance approximation (fast)
    double manhattan = std::abs(const_cast<Node*>(fromNode)->getCoordinate().getLatitude() - 
                                const_cast<Node*>(toNode)->getCoordinate().getLatitude()) +
                      std::abs(const_cast<Node*>(fromNode)->getCoordinate().getLongitude() - 
                                const_cast<Node*>(toNode)->getCoordinate().getLongitude());
    
    // Convert to meters (approximate)
    double meters = manhattan * 111000.0;
    
    // Scale to maintain admissibility
    return meters * HEURISTIC_SCALE;
}

bool AStarAlgorithm::isEdgeRestrictedForVehicle(const Edge& edge, const VehicleProfile* vehicleProfile) const {
    if (!vehicleProfile || edge.getTags().find("highway") == edge.getTags().end()) {
        return false;
    }
    
    std::string highway = edge.getTags().at("highway");
    return vehicleProfile->isHighwayBlocked(highway);
}

std::vector<int64_t> AStarAlgorithm::findPath(
    const Graph& graph,
    int64_t startNodeId,
    int64_t endNodeId
) {
    return findPath(graph, startNodeId, endNodeId, nullptr);
}

std::vector<int64_t> AStarAlgorithm::findPath(
    const Graph& graph,
    int64_t startNodeId,
    int64_t endNodeId,
    const VehicleProfile* vehicleProfile
) {
    auto startTime = std::chrono::high_resolution_clock::now();
    nodesExplored = 0;
    
    // Initialize data structures
    std::unordered_map<int64_t, double> gScore;  // g(n) - actual cost from start
    std::unordered_map<int64_t, double> fScore;  // f(n) - g(n) + h(n)
    std::unordered_map<int64_t, int64_t> cameFrom;
    std::unordered_map<int64_t, int64_t> cameFromEdge;  // Store edge IDs for path reconstruction
    std::unordered_set<int64_t> visited;
    
    std::priority_queue<QueueNode, std::vector<QueueNode>, std::greater<QueueNode>> openSet;
    
    // Initialize start node
    gScore[startNodeId] = 0.0;
    double initialH = calculateHeuristic(graph, startNodeId, endNodeId);
    fScore[startNodeId] = initialH;
    openSet.push({startNodeId, initialH});
    
    int expansions = 0;
    bool pathFound = false;
    
    while (!openSet.empty() && expansions < MAX_EXPANSIONS) {
        QueueNode current = openSet.top();
        openSet.pop();
        
        int64_t currentId = current.nodeId;
        
        if (visited.count(currentId)) continue;
        visited.insert(currentId);
        nodesExplored++;
        expansions++;
        
        // Goal reached
        if (currentId == endNodeId) {
            pathFound = true;
            break;
        }
        
        // Explore neighbors
        const std::vector<Edge*> neighbors = graph.getOutgoingEdges(currentId);
        
        for (const auto& edge : neighbors) {
            int64_t neighborId = edge->getTarget()->getId();
            
            if (visited.count(neighborId)) continue;
            
            // Check vehicle restrictions
            if (vehicleProfile && isEdgeRestrictedForVehicle(*edge, vehicleProfile)) {
                continue;
            }
            
            double weight = edge->getDistance().getMeters();
            double tentativeG = gScore[currentId] + weight;
            
            // Check if this path is better
            if (gScore.find(neighborId) == gScore.end() || tentativeG < gScore[neighborId]) {
                cameFrom[neighborId] = currentId;
                cameFromEdge[neighborId] = edge->getId();  // Store edge ID
                gScore[neighborId] = tentativeG;
                
                double h = calculateHeuristic(graph, neighborId, endNodeId);
                double f = tentativeG + h;
                fScore[neighborId] = f;
                
                openSet.push({neighborId, f});
            }
        }
    }
    
    // Reconstruct path (as edge IDs, not node IDs)
    std::vector<int64_t> path;
    
    if (!pathFound) {
        std::cout << "[A*][WARN] No path found. Expansions: " << expansions << std::endl;
    } else {
        int64_t current = endNodeId;
        while (current != startNodeId) {
            auto it = cameFromEdge.find(current);
            if (it == cameFromEdge.end()) {
                std::cout << "[A*][ERROR] Path reconstruction failed" << std::endl;
                path.clear();
                break;
            }
            
            int64_t edgeId = it->second;
            path.push_back(edgeId);
            
            // Move to previous node
            auto nodeIt = cameFrom.find(current);
            if (nodeIt == cameFrom.end()) {
                std::cout << "[A*][ERROR] cameFrom missing" << std::endl;
                path.clear();
                break;
            }
            current = nodeIt->second;
        }
        std::reverse(path.begin(), path.end());
        
        std::cout << "[A*][OK] Path found. Expansions: " << expansions 
                  << ", Length: " << path.size() << " edges" << std::endl;
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    executionTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
    
    return path;
}
