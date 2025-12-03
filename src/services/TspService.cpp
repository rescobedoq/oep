#include "TspService.h"
#include "../algorithms/tsp/TspMatrix.h"
#include "../algorithms/tsp/IGAlgorithm.h"
#include "../algorithms/factories/AlgorithmFactory.h"
#include "../algorithms/factories/TspAlgorithmFactory.h"
#include "../utils/exceptions/GraphException.h"
#include "../utils/exceptions/TspException.h"
#include <QtConcurrent/QtConcurrent>
#include <chrono>
#include <algorithm>
#include <iostream>

TspService::TspService(QObject* parent)
    : QObject(parent)
    , graph_(nullptr)
{}

void TspService::solveAsync(
    const std::vector<int64_t>& waypointIds,
    const std::string& tspAlgorithmName,
    const std::string& pathfindingAlgorithmName,
    const VehicleProfile* vehicleProfile,
    bool returnToStart
) {
    // Copiar VehicleProfile si existe (para evitar use-after-free en thread asíncrono)
    std::unique_ptr<VehicleProfile> vehicleProfileCopy = nullptr;
    if (vehicleProfile) {
        vehicleProfileCopy = std::make_unique<VehicleProfile>(*vehicleProfile);
    }
    
    // Run in Qt thread pool (thread-safe with Qt signals)
    tspFuture_ = QtConcurrent::run([this, waypointIds, tspAlgorithmName, pathfindingAlgorithmName, 
                                    vehicleProfileCopy = std::move(vehicleProfileCopy), returnToStart]() {
        try {
            if (!graph_) {
                throw GraphException("Graph not loaded");
            }
            
            // Input validation
            if (waypointIds.size() < 2) {
                throw TspException(
                    TspException::ErrorCode::INSUFFICIENT_NODES,
                    "TSP requires at least 2 waypoints. Provided: " + 
                    std::to_string(waypointIds.size())
                );
            }
            
            // Validate that nodes exist in the graph
            std::vector<int64_t> invalidNodes;
            for (int64_t nodeId : waypointIds) {
                if (!graph_->getNode(nodeId)) {
                    invalidNodes.push_back(nodeId);
                }
            }
            
            if (!invalidNodes.empty()) {
                throw TspException(
                    TspException::ErrorCode::INVALID_NODES,
                    std::to_string(invalidNodes.size()) + " waypoint(s) not found in graph",
                    invalidNodes
                );
            }
            
            auto totalStartTime = std::chrono::high_resolution_clock::now();
            
            // 1. Create TspMatrix
            size_t n = waypointIds.size();
            TspMatrix matrix(n, waypointIds);
            
            // 2. Precompute matrix (with progress callback)
            auto precomputeStartTime = std::chrono::high_resolution_clock::now();
            
            auto pathfindingAlgo = AlgorithmFactory::createAlgorithm(pathfindingAlgorithmName);
            
            matrix.precompute(
                *graph_,
                pathfindingAlgo.get(),
                vehicleProfileCopy.get(),  // Usar la copia
                [this](int current, int total, int percent) {
                    // Emit progress (thread-safe with Qt::QueuedConnection)
                    emit precomputeProgress(percent);
                }
            );
            
            auto precomputeEndTime = std::chrono::high_resolution_clock::now();
            double precomputeTimeMs = std::chrono::duration<double, std::milli>(
                precomputeEndTime - precomputeStartTime).count();
            
            // 2.5. VALIDATE that the matrix has a valid solution
            if (!matrix.hasValidSolution()) {
                // Get unreachable pairs for diagnostics
                auto unreachable = matrix.getUnreachablePairs();
                
                // Extract unique problematic nodes
                std::vector<int64_t> problematicNodes;
                for (const auto& pair : unreachable) {
                    int64_t fromNode = matrix.getNodeId(pair.first);
                    int64_t toNode = matrix.getNodeId(pair.second);
                    
                    if (std::find(problematicNodes.begin(), problematicNodes.end(), fromNode) 
                        == problematicNodes.end()) {
                        problematicNodes.push_back(fromNode);
                    }
                    if (std::find(problematicNodes.begin(), problematicNodes.end(), toNode) 
                        == problematicNodes.end()) {
                        problematicNodes.push_back(toNode);
                    }
                }
                
                std::string errorMsg = "TSP validation failed: " + 
                    std::to_string(unreachable.size()) + " unreachable pairs found.";
                
                if (vehicleProfileCopy) {
                    errorMsg += " Try using a different vehicle profile or removing restricted waypoints.";
                }
                
                throw TspException(
                    TspException::ErrorCode::UNREACHABLE_NODES,
                    errorMsg,
                    problematicNodes
                );
            }
            
            // 3. Solve TSP
            auto tspAlgo = TspAlgorithmFactory::create(tspAlgorithmName);
            
            // Configure returnToStart if IG
            if (auto igAlgo = dynamic_cast<IGAlgorithm*>(tspAlgo.get())) {
                igAlgo->setReturnToStart(returnToStart);
            }
            
            auto tspStartTime = std::chrono::high_resolution_clock::now();
            std::vector<int> tour = tspAlgo->solve(matrix, waypointIds);
            auto tspEndTime = std::chrono::high_resolution_clock::now();
            double tspTimeMs = std::chrono::duration<double, std::milli>(tspEndTime - tspStartTime).count();
            
            auto totalEndTime = std::chrono::high_resolution_clock::now();
            double totalTimeMs = std::chrono::duration<double, std::milli>(
                totalEndTime - totalStartTime).count();
            
            std::cout << "   TSP Algorithm computation time: " << tspTimeMs << " ms" << std::endl;
            std::cout << "   Total time (matrix + TSP): " << totalTimeMs << " ms" << std::endl;
            
            // 4. Calculate total distance and collect segment details
            double totalDistance = matrix.calculateTourCost(tour, returnToStart);
            
            // ✅ Collect edges and nodes for each segment
            std::vector<std::vector<Edge*>> segmentEdges;
            std::vector<std::vector<int64_t>> segmentNodes;
            
            size_t numSegments = returnToStart ? tour.size() : tour.size() - 1;
            
            for (size_t i = 0; i < numSegments; ++i) {
                int fromIdx = tour[i];
                int toIdx = tour[(i + 1) % tour.size()];
                
                int64_t fromNodeId = waypointIds[fromIdx];
                int64_t toNodeId = waypointIds[toIdx];
                
                // Get the path for this segment from the matrix
                auto segmentPath = matrix.getPath(fromIdx, toIdx);
                
                std::vector<Edge*> edges;
                std::vector<int64_t> nodes;
                
                // Add start node
                if (!segmentPath.empty()) {
                    Edge* firstEdge = graph_->getEdge(segmentPath[0]);
                    if (firstEdge && firstEdge->getSource()) {
                        nodes.push_back(firstEdge->getSource()->getId());
                    }
                }
                
                // Collect edges and nodes
                for (int64_t edgeId : segmentPath) {
                    Edge* edge = graph_->getEdge(edgeId);
                    if (edge) {
                        edges.push_back(edge);
                        if (edge->getTarget()) {
                            nodes.push_back(edge->getTarget()->getId());
                        }
                    }
                }
                
                segmentEdges.push_back(edges);
                segmentNodes.push_back(nodes);
            }
            
            // 5. Build result
            TspResult result;
            result.tour = tour;
            result.nodeIds = waypointIds;
            result.segmentEdges = segmentEdges;   // ✅ Edges por segmento
            result.segmentNodes = segmentNodes;   // ✅ Nodos por segmento
            result.totalDistance = totalDistance;
            result.executionTimeMs = totalTimeMs;
            result.precomputeTimeMs = precomputeTimeMs;
            result.tspAlgorithmName = tspAlgorithmName;
            
            emit tspSolved(result);
            
        } catch (const TspException& e) {
            // Specific TSP error with detailed information
            QString errorMsg = QString::fromStdString(e.getUserFriendlyMessage());
            
            // Add recovery suggestions
            auto suggestions = e.getRecoverySuggestions();
            if (!suggestions.empty()) {
                errorMsg += "\n\nSuggestions:";
                for (const auto& suggestion : suggestions) {
                    errorMsg += "\n  • " + QString::fromStdString(suggestion);
                }
            }
            
            emit tspError(errorMsg);
            
        } catch (const GraphException& e) {
            // Graph error
            emit tspError(QString("Graph error: %1").arg(e.what()));
            
        } catch (const std::exception& e) {
            // Generic error
            emit tspError(QString("TSP error: %1").arg(e.what()));
        }
    });
}
