#pragma once

#include "BaseException.h"
#include <exception>
#include <string>

/**
 * Exception for graph-related errors
 */

class GraphException : public BaseException {
public:
    explicit GraphException(const std::string& message)
        : BaseException("Graph Error: " + message) {}
};

class NodeNotFoundException : public GraphException {
public:
    explicit NodeNotFoundException(int64_t nodeId)
        : GraphException("Node not found: " + std::to_string(nodeId)) {}
};

class PathNotFoundException : public GraphException {
public:
    explicit PathNotFoundException(int64_t fromId, int64_t toId)
        : GraphException("Path not found from node " + std::to_string(fromId) +
                         " to node " + std::to_string(toId)) {}
};

class GraphLoaderException : public GraphException {
public:
    explicit GraphLoaderException(const std::string& filePath)
        : GraphException("Graph Loader Error: " + filePath) {}
};