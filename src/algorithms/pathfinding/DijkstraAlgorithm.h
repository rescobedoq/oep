#include "../../core/interfaces/IPathfindingAlgorithm.h"
#include "../../core/entities/Graph.h"
#include "../VehicleProfile.h"

class DijkstraAlgorithm : public IPathfindingAlgorithm {
private:
    size_t nodesExplored = 0;
    double executionTime = 0.0;

    struct QueueNode {
        int64_t nodeId;
        double cost;
        
        bool operator>(const QueueNode& other) const {
            return cost > other.cost;
        }
    };

public:
    DijkstraAlgorithm() : nodesExplored(0), executionTime(0.0) {}

    std::vector<int64_t> findPath(
        const Graph& graph, 
        int64_t startNodeId, 
        int64_t endNodeId
    ) override;

    std::vector<int64_t> findPath(
        const Graph& graph, 
        int64_t startNodeId, 
        int64_t endNodeId,
        const VehicleProfile* vehicleProfile
    ) override;

    bool isEdgeRestrictedForVehicle(
        const Edge& edge,
        const VehicleProfile* vehicleProfile
    ) const;

    std::string getName() const override { return "dijkstra"; }
    size_t getNodesExplored() const override { return nodesExplored; }
    double getExecutionTime() const override { return executionTime; }
};