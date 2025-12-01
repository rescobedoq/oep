#include "TspMatrix.h"
#include <limits>
#include <unordered_set>
#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <algorithm>
#include <cmath>

TspMatrix::TspMatrix(size_t size, const std::vector<int64_t>& nodeIds)
    : size_(size)
    , nodeIds_(nodeIds)
{
    // Initialize N x N matrix
    matrix_.resize(size_);
    for (size_t i = 0; i < size_; i++) {
        matrix_[i].resize(size_);
    }
}

void TspMatrix::precompute(
    const Graph& graph,
    IPathfindingAlgorithm* algorithm,
    const VehicleProfile* vehicleProfile,
    ProgressCallback progressCallback
) {
    std::cout << "Starting parallel TSP matrix" << std::endl;
    std::cout << "   - Size: " << size_ << "x" << size_ << std::endl;
    std::cout << "   - Algorithm: " << algorithm->getName() << std::endl;
    
    std::atomic<int> completedRows{0};
    std::mutex progressMutex;
    
    // Determine number of threads
    unsigned int numThreads = std::min(
        static_cast<unsigned int>(std::thread::hardware_concurrency()),
        static_cast<unsigned int>(size_)
    );
    if (numThreads == 0) numThreads = 4; // fallback
    
    std::cout << "   - " << numThreads << " threads for " << size_ << " nodes" << std::endl;
    
    // STRATEGY: One thread processes FULL ROWS
    // This drastically reduces synchronization overhead
    auto processRow = [&](size_t rowIdx) {
        int64_t fromId = nodeIds_[rowIdx];
        
        for (size_t j = 0; j < size_; j++) {
            if (rowIdx == j) {
                // Distancia a sí mismo = 0
                matrix_[rowIdx][j] = Entry(0.0, {});
            } else {
                // Calcular shortest path
                int64_t toId = nodeIds_[j];
                
                std::vector<int64_t> path;
                if (vehicleProfile) {
                    path = algorithm->findPath(graph, fromId, toId, vehicleProfile);
                } else {
                    path = algorithm->findPath(graph, fromId, toId);
                }
                
                double distance = 0.0;
                if (!path.empty()) {
                    distance = calculatePathDistance(graph, path);
                } else {
                    distance = std::numeric_limits<double>::infinity();
                }
                
                matrix_[rowIdx][j] = Entry(distance, path);
            }
        }
        
        // Report progress upon completing the row
        int completed = ++completedRows;
        std::cout << "TSP Matrix: " << completed << "/" << size_ << " rows" << std::endl;
        
        if (progressCallback) {
            std::lock_guard<std::mutex> lock(progressMutex);
            int totalPairs = size_ * size_;
            int completedPairs = completed * size_;
            int percent = (completedPairs * 100) / totalPairs;
            progressCallback(completedPairs, totalPairs, percent);
        }
    };
    
    // Throw threads for processing rows
    std::vector<std::thread> threads;
    std::atomic<size_t> nextRow{0};
    
    auto workerFunction = [&]() {
        while (true) {
            size_t row = nextRow.fetch_add(1);
            if (row >= size_) break;
            processRow(row);
        }
    };
    
    for (unsigned int t = 0; t < numThreads; t++) {
        threads.emplace_back(workerFunction);
    }
    
    // Wait for all threads to finish
    for (auto& thread : threads) {
        thread.join();
    }
    
    std::cout << "TSP Matrix completed" << std::endl;
}

const TspMatrix::Entry& TspMatrix::getEntry(int64_t fromNodeId, int64_t toNodeId) const {
    size_t fromIdx = getNodeIndex(fromNodeId);
    size_t toIdx = getNodeIndex(toNodeId);
    return matrix_[fromIdx][toIdx];
}

size_t TspMatrix::getNodeIndex(int64_t nodeId) const {
    for (size_t i = 0; i < nodeIds_.size(); i++) {
        if (nodeIds_[i] == nodeId) {
            return i;
        }
    }
    throw std::runtime_error("Node ID not found in TspMatrix");
}

double TspMatrix::calculateTourCost(const std::vector<int>& tour, bool returnToStart) const {
    double totalCost = 0.0;
    
    for (size_t i = 0; i < tour.size() - 1; i++) {
        totalCost += matrix_[tour[i]][tour[i + 1]].distance;
    }
    
    if (returnToStart && tour.size() > 1) {
        totalCost += matrix_[tour.back()][tour.front()].distance;
    }
    
    return totalCost;
}

std::vector<int> TspMatrix::nearestNeighborRoute(int startIdx) const {
    std::vector<int> route;
    std::unordered_set<int> remaining;
    
    for (int i = 0; i < static_cast<int>(size_); i++) {
        remaining.insert(i);
    }
    
    int current = startIdx;
    route.push_back(current);
    remaining.erase(current);
    
    while (!remaining.empty()) {
        int nearest = -1;
        double minDist = std::numeric_limits<double>::max();
        
        for (int candidate : remaining) {
            double dist = matrix_[current][candidate].distance;
            if (dist < minDist) {
                minDist = dist;
                nearest = candidate;
            }
        }
        
        route.push_back(nearest);
        remaining.erase(nearest);
        current = nearest;
    }
    
    return route;
}

double TspMatrix::calculatePathDistance(const Graph& graph, const std::vector<int64_t>& edgeIds) const {
    double totalDistance = 0.0;
    
    for (int64_t edgeId : edgeIds) {
        Edge* edge = graph.getEdge(edgeId);
        if (edge) {
            totalDistance += edge->getDistance().getMeters();
        }
    }
    
    return totalDistance;
}

std::vector<std::pair<size_t, size_t>> TspMatrix::getUnreachablePairs() const {
    std::vector<std::pair<size_t, size_t>> unreachable;
    
    for (size_t i = 0; i < size_; i++) {
        for (size_t j = 0; j < size_; j++) {
            if (i != j && std::isinf(matrix_[i][j].distance)) {
                unreachable.push_back({i, j});
            }
        }
    }
    
    return unreachable;
}

bool TspMatrix::hasValidSolution() const {
    // For TSP we need all nodes to be reachable from each other
    // (strongly connected graph in terms of waypoints)
    
    for (size_t i = 0; i < size_; i++) {
        for (size_t j = 0; j < size_; j++) {
            if (i != j && std::isinf(matrix_[i][j].distance)) {
                return false;
            }
        }
    }
    
    return true;
}

