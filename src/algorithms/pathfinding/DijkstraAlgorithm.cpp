#include "DijkstraAlgorithm.h"
#include "../../utils/exceptions/GraphException.h"
#include <memory>
#include <queue>
#include <unordered_set>
#include <limits>
#include <algorithm>

std::vector<int64_t> DijkstraAlgorithm::findPath(
    const Graph& graph, 
    int64_t startNodeId, 
    int64_t endNodeId
) {
    return findPath(graph, startNodeId, endNodeId, nullptr);
}

std::vector<int64_t> DijkstraAlgorithm::findPath(
    const Graph& graph, 
    int64_t startNodeId, 
    int64_t endNodeId,
    const VehicleProfile* vehicleProfile
) {
    nodesExplored = 0;
    executionTime = 0.0;
    
    // Verification of nodes existence
    if (!graph.hasNode(startNodeId)) {
        throw GraphException("Start node not found in graph");
    }
    if (!graph.hasNode(endNodeId)) {
        throw GraphException("End node not found in graph");
    }

    // Dijkstra's algorithm initialization
    std::unordered_map<int64_t, double> distances;
    std::unordered_map<int64_t, int64_t> previousEdge;
    std::unordered_set<int64_t> visitedNodes;
    std::priority_queue<QueueNode, std::vector<QueueNode>, std::greater<QueueNode>> priorityQueue;

    // OPTIMIZATION: Do not initialize ALL nodes, only those we explore
    // Reserve estimated space to avoid reallocations
    distances.reserve(10000);
    previousEdge.reserve(10000);
    visitedNodes.reserve(10000);
    
    distances[startNodeId] = 0.0;
    priorityQueue.push({startNodeId, 0.0});

    // Main loop
    while (!priorityQueue.empty()) {
        QueueNode current = priorityQueue.top();
        priorityQueue.pop();

        if (visitedNodes.count(current.nodeId) > 0) {
            continue; // Already processed
        }
        visitedNodes.insert(current.nodeId);
        nodesExplored++;

        // If we reached the destination
        if (current.nodeId == endNodeId) {
            break;
        }

        // Explore neighbors
        std::vector<Edge*> outgoingEdges = graph.getOutgoingEdges(current.nodeId);
        
        for (Edge* edge : outgoingEdges) {
            if (vehicleProfile != nullptr && isEdgeRestrictedForVehicle(*edge, vehicleProfile)) {
                continue; // Skip restricted edges
            }

            int64_t neighborId = edge->getTarget()->getId();
            double newDist = distances[current.nodeId] + edge->getDistance().getMeters();

            // OPTIMIZATION: Only check if it does not exist or is better
            auto it = distances.find(neighborId);
            if (it == distances.end() || newDist < it->second) {
                distances[neighborId] = newDist;
                previousEdge[neighborId] = edge->getId();
                priorityQueue.push({neighborId, newDist});
            }
        }
    }
    
    // Reconstruct path
    std::vector<int64_t> path;
    
    // If end node was not reached
    if (previousEdge.find(endNodeId) == previousEdge.end() && startNodeId != endNodeId) {
        return path; // Empty path = no route found
    }
    
    // Build path backwards
    int64_t currentNode = endNodeId;
    while (currentNode != startNodeId) {
        auto it = previousEdge.find(currentNode);
        if (it == previousEdge.end()) {
            break;
        }
        
        int64_t edgeId = it->second;
        path.push_back(edgeId);
        
        Edge* edge = graph.getEdge(edgeId);
        if (!edge) {
            break;
        }
        currentNode = edge->getSource()->getId();
    }
    
    // Reverse to get correct order (start -> end)
    std::reverse(path.begin(), path.end());
    
    return path;
}

bool DijkstraAlgorithm::isEdgeRestrictedForVehicle(
        const Edge& edge,
        const VehicleProfile* vehicleProfile
) const {
    if (vehicleProfile == nullptr) {
        return false; // No restrictions
    }

    const auto& tags = edge.getTags();
    
    if (tags.count("highway")) {
        std::string highwayType = tags.at("highway");
        
        if (vehicleProfile->isHighwayBlocked(highwayType)) {
            return true; // Blocked highway type
        }
    }

    return !vehicleProfile->isRoadSuitable(tags);
}