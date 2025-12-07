#include <vector>
#include "Graph.h"

Graph::Graph() : minLatitude(0), maxLatitude(0), minLongitude(0), maxLongitude(0), boundsSet(false) {}

void Graph::addNode(
    int64_t id,
    double lat,
    double lon
) {
    nodes[id] = std::make_unique<Node>(id, Coordinate(lat, lon));
}

void Graph::addEdge(
    int64_t id,
    int64_t fromId,
    int64_t toId,
    const Distance& distance,
    bool isOneWay,
    const std::unordered_map<std::string, std::string>& tags
) {
    Node* from = getNode(fromId);
    Node* to = getNode(toId);

    if (!from || !to)
        throw std::invalid_argument("Cannot create edge: one or both node IDs do not exist.");

    edges[id] = std::make_unique<Edge>(id, from, to, isOneWay, distance, tags);
}

void Graph::buildAdjacencyList() {
    adjacencyList.clear();
    
    for (const auto& [id, edgePtr] : edges) {
        Edge* edge = edgePtr.get();
        adjacencyList[edge->getSource()->getId()].push_back(edge);
        if (!edge->IsOneWay()) {
            adjacencyList[edge->getTarget()->getId()].push_back(edge);
        }
    }
}

Node* Graph::getNode(int64_t id) const {
    auto it = nodes.find(id);
    return (it != nodes.end()) ? it->second.get() : nullptr;
}

std::vector<Node*> Graph::getNodes() const {
    std::vector<Node*> result;
    result.reserve(nodes.size());
    for (const auto& [id, nodePtr] : nodes) {
        result.push_back(nodePtr.get());
    }
    return result;
}

bool Graph::hasNode(int64_t id) const {
    return nodes.count(id) > 0;
}

size_t Graph::getNodeCount() const {
    return nodes.size();
}

Edge* Graph::getEdge(int64_t id) const {
    auto it = edges.find(id);
    return (it != edges.end()) ? it->second.get() : nullptr;
}

std::vector<Edge*> Graph::getEdges() const {
    std::vector<Edge*> result;
    result.reserve(edges.size());
    for (const auto& [id, edgePtr] : edges) {
        result.push_back(edgePtr.get());
    }
    return result;
}


bool Graph::hasEdge(int64_t id) const {
    return edges.count(id) > 0;
}

size_t Graph::getEdgeCount() const {
    return edges.size();
}

std::vector<Edge*> Graph::getOutgoingEdges(int64_t nodeId) const {
    auto it = adjacencyList.find(nodeId);
    return (it != adjacencyList.end()) ? it->second : std::vector<Edge*>{};
}

std::vector<Node*> Graph::getNeighbors(int64_t nodeId) const {
    std::vector<Node*> neighbors;
    auto it = adjacencyList.find(nodeId);
    if (it != adjacencyList.end()) {
        for (const Edge* edge : it->second) {
            Node* neighbor = (edge->getSource()->getId() == nodeId)
                                ? edge->getTarget()
                                : edge->getSource();
            neighbors.push_back(neighbor);
        }
    }
    return neighbors;
}

bool Graph::hasDirectEdge(int64_t fromId, int64_t toId) const {
    auto edgeIds = getOutgoingEdges(fromId);
    for (const Edge* edge : edgeIds) {
        if (edge->getTarget()->getId() == toId ||
            (!edge->IsOneWay() && edge->getSource()->getId() == toId)) {
            return true;
        }
    }
    return false;
}

void Graph::setBounds( double minLat, double maxLat, double minLon, double maxLon) {
    minLatitude = minLat;
    maxLatitude = maxLat;
    minLongitude = minLon;
    maxLongitude = maxLon;
    boundsSet = true;
}

bool Graph::isWithinBounds(double lat, double lon) const {
    if (!boundsSet) return true;
    return lat >= minLatitude && lat <= maxLatitude && lon >= minLongitude && lon <= maxLongitude;
}

std::tuple<double, double, double, double> Graph::getBounds() const {
    return std::make_tuple(minLatitude, maxLatitude, minLongitude, maxLongitude);
}

void Graph::clear() {
    nodes.clear();
    edges.clear();
    adjacencyList.clear();
    boundsSet = false;
}