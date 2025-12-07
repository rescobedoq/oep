#include "gtest/gtest.h"
#include "../../src/algorithms/pathfinding/DijkstraAlgorithm.h"
#include "../../src/core/entities/Graph.h"
#include "../../src/core/entities/Node.h" 
#include "../../src/core/entities/Edge.h" 
#include "../../src/core/value_objects/Distance.h" 
#include "../../src/core/value_objects/Coordinate.h" 
#include "../../src/algorithms/VehicleProfile.h" 

class DijkstraTest : public ::testing::Test {
protected:
    Graph testGraph;
    DijkstraAlgorithm dijkstra; 
    VehicleProfile carProfile{"Car", "driving"};
    
    // Función auxiliar para obtener el coste total del camino 
    double calculatePathCost(const Graph& graph, const std::vector<int64_t>& path) const {
        double totalDistance = 0.0;
        for (int64_t edgeId : path) {
            const Edge* edge = graph.getEdge(edgeId);
            if (edge) {
                totalDistance += edge->getDistance().getMeters(); 
            }
        }
        return totalDistance;
    }

    void SetUp() override {
        testGraph.addNode(10, 0, 0); // Node A (10)
        testGraph.addNode(20, 0, 1); // Node B (20)
        testGraph.addNode(30, 1, 0); // Node C (30)
        testGraph.addNode(40, 1, 1); // Node D (40)
        testGraph.addNode(50, 2, 2); // Node E (50)
        
        testGraph.addEdge(100, 10, 20, Distance(4.0)); 
        testGraph.addEdge(101, 10, 30, Distance(1.0)); 
        testGraph.addEdge(102, 20, 40, Distance(2.0)); 
        testGraph.addEdge(103, 30, 40, Distance(5.0)); 

        testGraph.addEdge(104, 20, 10, Distance(4.0)); 
        testGraph.addEdge(105, 30, 10, Distance(1.0)); 
        testGraph.addEdge(106, 40, 20, Distance(2.0)); 
        testGraph.addEdge(107, 40, 30, Distance(5.0)); 

        testGraph.addEdge(200, 20, 50, Distance(1.0)); 
        testGraph.addEdge(201, 50, 20, Distance(1.0)); 
        std::unordered_map<std::string, std::string> restrictedTags = {
            {"highway", "private"} // Usamos un tag 'highway' para activar la restricción
        };
        testGraph.addEdge(202, 30, 50, Distance(6.0), false, restrictedTags); 
        testGraph.addEdge(203, 50, 30, Distance(6.0)); 

        // Bloquea el tipo de vía "private" asignándole un speedFactor de 0.0.
        carProfile.setSpeedFactor("private", 0.0); 
        testGraph.buildAdjacencyList();
    }
};

TEST_F(DijkstraTest, ShortestPath10_to_40) {
    std::vector<int64_t> path = dijkstra.findPath(testGraph, 10, 40);
    ASSERT_FALSE(path.empty()) << "El camino de 10 a 40 no debería estar vacío";
    double totalDistance = calculatePathCost(testGraph, path);
    EXPECT_NEAR(totalDistance, 6.0, 1e-6) << "El costo debe ser 6.0";
    EXPECT_EQ(path.size(), 2) << "El camino debe tener 2 aristas";
}

TEST_F(DijkstraTest, DirectPath10_to_30) {
    std::vector<int64_t> path = dijkstra.findPath(testGraph, 10, 30);    
    ASSERT_EQ(path.size(), 1) << "El camino directo de 10 a 30 debe tener 1 arista";
    EXPECT_EQ(path[0], 101) << "El ID de la arista debe ser 101";
    double totalDistance = calculatePathCost(testGraph, path);
    EXPECT_NEAR(totalDistance, 1.0, 1e-6) << "El costo debe ser 1.0";
}

TEST_F(DijkstraTest, NoPathToUnconnectedNode) {
    testGraph.addNode(60, 10, 10);
    std::vector<int64_t> path = dijkstra.findPath(testGraph, 10, 60);    
    EXPECT_TRUE(path.empty()) << "No debe haber camino a un nodo desconectado";
}

TEST_F(DijkstraTest, StartAndEndAreSame) {
    std::vector<int64_t> path = dijkstra.findPath(testGraph, 10, 10);    
    EXPECT_TRUE(path.empty()) << "El camino del nodo a sí mismo debe estar vacío";
}

TEST_F(DijkstraTest, RestrictedPathVehicleProfile) {
    std::vector<int64_t> path = dijkstra.findPath(testGraph, 10, 50, &carProfile);   
    ASSERT_FALSE(path.empty()) << "Debe existir un camino alternativo (A->B->E)";   
    double totalDistance = calculatePathCost(testGraph, path);

    EXPECT_NEAR(totalDistance, 5.0, 1e-6) << "El costo del camino más corto debe ser 5.0, evitando la arista restringida (7.0)";
    EXPECT_EQ(path.size(), 2) << "El camino debe ser de 2 aristas";
    EXPECT_TRUE((path[0] == 100 && path[1] == 200)) << "El camino debe ser 100 -> 200 (A->B->E)";

    bool tookRestrictedEdge = false;
    for (int64_t edgeId : path) {
        if (edgeId == 202) {
            tookRestrictedEdge = true;
            break;
        }
    }
    EXPECT_FALSE(tookRestrictedEdge) << "El camino no debe incluir la arista restringida 202";
}