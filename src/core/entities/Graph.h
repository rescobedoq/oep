#pragma once

#include <unordered_map>
#include <vector>
#include <memory>
#include "Node.h"
#include "Edge.h"

class Graph {
private:
    std::unordered_map<int64_t, std::unique_ptr<Node>> nodes;
    std::unordered_map<int64_t, std::unique_ptr<Edge>> edges;

    // Adjacency list: node ID to list of outgoing edges
    std::unordered_map<int64_t, std::vector<Edge*>> adjacencyList;

    // Bounding box (limits of the graph)
    double minLatitude, maxLatitude, minLongitude, maxLongitude;
    bool boundsSet = false;

public:
    Graph();


    // Construction of the graph
    void addNode(
        int64_t id,
        double lat = 0,
        double lon = 0
    );
    void addEdge(
        int64_t id,
        int64_t fromId,
        int64_t toId,
        const Distance& distance = Distance(0),
        bool isOneWay = false,
        const std::unordered_map<std::string, std::string>& tags = {}
    );
    void buildAdjacencyList();

    // Consults - Nodes
    Node* getNode(int64_t id) const;
    std::vector<Node*> getNodes() const;  // Returns vector of raw pointers
    bool hasNode(int64_t id) const;
    size_t getNodeCount() const;
    
    // Direct access to internal maps (for serialization only - use with caution)
    const std::unordered_map<int64_t, std::unique_ptr<Node>>& getNodesMap() const { return nodes; }

    // Consults - Edges
    Edge* getEdge(int64_t id) const;
    std::vector<Edge*> getEdges() const;  // Returns vector of raw pointers
    bool hasEdge(int64_t id) const;
    size_t getEdgeCount() const;
    
    // Direct access to internal maps (for serialization only - use with caution)
    const std::unordered_map<int64_t, std::unique_ptr<Edge>>& getEdgesMap() const { return edges; }

    // Adyacencia
    std::vector<Edge*> getOutgoingEdges(int64_t nodeId) const;      // Useful for pathfinding
    std::vector<Node*> getNeighbors(int64_t nodeId) const;
    bool hasDirectEdge(int64_t fromId, int64_t toId) const;

    // Bounding box
    bool hasBoundSet() const { return boundsSet; }
    void setBounds(double minLat, double maxLat, double minLon, double maxLon);
    bool isWithinBounds(double lat, double lon) const;
    std::tuple<double, double, double, double> getBounds() const;
    
    // Utilidades
    void clear();
    bool isEmpty() const { return nodes.empty(); }
};