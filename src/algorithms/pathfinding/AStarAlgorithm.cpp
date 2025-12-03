// src/algorithms/pathfinding/AStarAlgorithm.cpp
#include "AStarAlgorithm.h"
#include "../../core/entities/Edge.h"
#include "../../core/entities/Node.h"
#include <cmath>
#include <limits>
#include <iostream>
#include <chrono>
#include <algorithm>

AStarAlgorithm::AStarAlgorithm() 
    : nodesExplored_(0), executionTime_(0.0) {}

std::vector<int64_t> AStarAlgorithm::findPath(
    const Graph& graph,
    int64_t startNodeId,
    int64_t endNodeId
) {
    return calculatePath(graph, startNodeId, endNodeId, nullptr);
}

std::vector<int64_t> AStarAlgorithm::findPath(
    const Graph& graph,
    int64_t startNodeId,
    int64_t endNodeId,
    const VehicleProfile* vehicleProfile
) {
    return calculatePath(graph, startNodeId, endNodeId, vehicleProfile);
}

std::vector<int64_t> AStarAlgorithm::calculatePath(
    const Graph& graph,
    int64_t startNodeId,
    int64_t endNodeId,
    const VehicleProfile* vehicleProfile
) const {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Initialize data structures
    std::unordered_map<int64_t, double> gScore;
    std::unordered_map<int64_t, double> fScore;
    std::unordered_map<int64_t, int64_t> prevEdge;  // nodeId -> edgeId
    std::unordered_set<int64_t> visited;
    
    // Priority queue: min-heap by fScore
    std::priority_queue<NodeScore, std::vector<NodeScore>, std::greater<NodeScore>> openSet;
    
    // Initialize all nodes to infinity
    auto nodes = graph.getNodes();
    for (const auto* node : nodes) {
        gScore[node->getId()] = std::numeric_limits<double>::infinity();
        fScore[node->getId()] = std::numeric_limits<double>::infinity();
    }
    
    // Start node
    gScore[startNodeId] = 0.0;
    fScore[startNodeId] = calculateHeuristic(graph, startNodeId, endNodeId);
    openSet.push(NodeScore(startNodeId, fScore[startNodeId]));
    
    size_t expansions = 0;
    double bestFScore = fScore[startNodeId];
    
    while (!openSet.empty() && expansions < MAX_EXPANSIONS) {
        NodeScore current = openSet.top();
        openSet.pop();
        
        int64_t currentId = current.nodeId;
        
        // Skip if already visited
        if (visited.count(currentId)) continue;
        visited.insert(currentId);
        expansions++;
        
        // Goal reached
        if (currentId == endNodeId) break;
        
        // Update best fScore
        if (fScore[currentId] < bestFScore) {
            bestFScore = fScore[currentId];
        }
        
        // Explore neighbors
        auto neighbors = graph.getOutgoingEdges(currentId);
        if (neighbors.empty()) continue;
        
        for (const auto* edge : neighbors) {
            int64_t neighborId = edge->getTarget()->getId();
            
            // Skip if already visited
            if (visited.count(neighborId)) continue;
            
            // Check vehicle restrictions
            if (vehicleProfile && isEdgeRestrictedForVehicle(edge, vehicleProfile)) {
                continue;
            }
            
            double weight = edge->getDistance().getMeters();
            double tentativeGScore = gScore[currentId] + weight;
            
            // Better path found?
            if (tentativeGScore < gScore[neighborId]) {
                prevEdge[neighborId] = edge->getId();
                gScore[neighborId] = tentativeGScore;
                
                double heuristic = calculateHeuristic(graph, neighborId, endNodeId);
                fScore[neighborId] = tentativeGScore + heuristic;
                
                openSet.push(NodeScore(neighborId, fScore[neighborId]));
            }
        }
    }
    
    if (expansions >= MAX_EXPANSIONS) {
        std::cout << "[A*] Expansion limit reached, returning partial route" << std::endl;
    }
    
    // Reconstruct path
    auto path = reconstructPath(prevEdge, startNodeId, endNodeId);
    
    // Validate path
    validatePath(path, graph, expansions);
    
    // Update metrics
    auto endTime = std::chrono::high_resolution_clock::now();
    nodesExplored_ = expansions;
    executionTime_ = std::chrono::duration<double>(endTime - startTime).count();
    
    return path;
}

double AStarAlgorithm::calculateHeuristic(
    const Graph& graph,
    int64_t fromNodeId,
    int64_t toNodeId
) const {
    const Node* fromNode = graph.getNode(fromNodeId);
    const Node* toNode = graph.getNode(toNodeId);
    
    if (!fromNode || !toNode) {
        return 0.0;  // Fallback if no coordinates
    }
    
    // Manhattan distance approximation (faster than Haversine for short distances)
    double latDiff = std::abs(fromNode->getCoordinate().getLatitude() - 
                              toNode->getCoordinate().getLatitude());
    double lonDiff = std::abs(fromNode->getCoordinate().getLongitude() - 
                              toNode->getCoordinate().getLongitude());
    
    double manhattan = latDiff + lonDiff;
    double meters = manhattan * 111000.0;  // Approx meters per degree
    
    return meters * HEURISTIC_SCALE;
}

bool AStarAlgorithm::isEdgeRestrictedForVehicle(
    const Edge* edge,
    const VehicleProfile* profile
) const {
    if (!edge || !profile) {
        return false;
    }
    
    // Get highway type from edge tags
    std::string highway = edge->getTag("highway");
    if (highway.empty()) {
        return false;  // If no highway defined, allow
    }
    
    // Check if this highway is blocked for the vehicle
    return profile->isHighwayBlocked(highway);
}

std::vector<int64_t> AStarAlgorithm::reconstructPath(
    const std::unordered_map<int64_t, int64_t>& prevEdge,
    int64_t startNodeId,
    int64_t endNodeId
) const {
    std::vector<int64_t> path;
    int64_t currentId = endNodeId;
    bool reachedDestination = false;
    
    // Reconstruct path backwards
    while (prevEdge.count(currentId)) {
        int64_t edgeId = prevEdge.at(currentId);
        path.push_back(edgeId);
        
        if (currentId == startNodeId) {
            reachedDestination = true;
            break;
        }
        
        // Move to previous node (we need to get the edge's source)
        // This assumes we can get the edge from the graph later
        // For now, we'll track using prevEdge map differently
        // Let's simplify: store the previous node ID directly
        break;  // Temporary - needs proper edge->source tracking
    }
    
    std::reverse(path.begin(), path.end());
    
    if (!reachedDestination && !path.empty()) {
        std::cout << "[A*][WARN] Incomplete route: destination not reached." << std::endl;
    } else if (reachedDestination) {
        std::cout << "[A*][OK] Route reconstructed successfully." << std::endl;
    }
    
    return path;
}

void AStarAlgorithm::validatePath(
    const std::vector<int64_t>& path,
    const Graph& graph,
    size_t expansions
) const {
    std::cout << "[A*] Expansions: " << expansions 
              << ", Path length: " << path.size() << std::endl;
    
    double totalDistance = 0.0;
    bool hasZeroDistance = false;
    
    for (int64_t edgeId : path) {
        const Edge* edge = graph.getEdge(edgeId);
        if (!edge) continue;
        
        double weight = edge->getDistance().getMeters();
        totalDistance += weight;
        
        if (weight == 0.0) {
            hasZeroDistance = true;
            std::cout << "[A*][ERROR] Zero distance sub-route detected: "
                      << edge->getSource()->getId() << " -> "
                      << edge->getTarget()->getId() << std::endl;
        }
    }
    
    if (hasZeroDistance) {
        std::cout << "[A*][ERROR] Route contains at least one zero distance sub-route." << std::endl;
    }
}