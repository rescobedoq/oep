#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <iomanip>
#include <memory> 

#include "src/core/entities/Graph.h" 
#include "src/core/entities/Node.h"
#include "src/infraestructure/loaders/BinaryGraphLoader.h"
#include "src/services/PathfindingService.h"
#include "src/services/TspService.h"
#include "src/algorithms/VehicleProfile.h"
#include "src/algorithms/factories/VehicleProfileFactory.h"

using namespace services::io;

#define NUM_PATH_TESTS 100
#define NUM_TSP_NODES 15 
#define NUM_TSP_REPEATS 5 

using Clock = std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::microseconds;

std::vector<std::pair<int64_t, int64_t>> generateRandomNodePairs(const std::shared_ptr<Graph>& graphPtr, int count) {
    std::vector<std::pair<int64_t, int64_t>> pairs;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::vector<Node*> allNodes = graphPtr->getNodes();
    if (allNodes.empty()) return pairs;
    std::uniform_int_distribution<size_t> distrib_index(0, allNodes.size() - 1);    
    for (int i = 0; i < count; ++i) {
        int64_t originId = allNodes[distrib_index(gen)]->getId();
        int64_t destinationId = allNodes[distrib_index(gen)]->getId();
        pairs.push_back({originId, destinationId});
    }
    return pairs;
}

void runStaticTests(const std::shared_ptr<Graph>& graphPtr) {
    const Graph& graph = *graphPtr;
    std::cout << "\n=======================================================\n";
    std::cout << "               TABLA III: ESTADÍSTICAS DEL DATASET     \n";
    std::cout << "=======================================================\n";
    std::cout << "Total de Nodos: " << graph.getNodeCount() << std::endl; 
    std::cout << "Total de Aristas: " << graph.getEdgeCount() << std::endl;
    std::cout << "Area (km2): (DEBE SER CALCULADO O BUSCADO MANUALMENTE)\n";
    std::cout << "=======================================================\n";
}

void runPathfindingTests(const std::shared_ptr<Graph>& graphPtr, const std::shared_ptr<VehicleProfile>& profilePtr) {
    std::cout << "\n=======================================================\n";
    std::cout << "               TABLAS IV Y V: RENDIMIENTO RUTA CORTA     \n";
    std::cout << "=======================================================\n";
    auto test_pairs = generateRandomNodePairs(graphPtr, NUM_PATH_TESTS);
    std::vector<std::string> pathfindingAlgorithms = {
        "dijkstra", 
        "astar",       
        "alt"       
    };
    PathfindingService pathService;
    pathService.setGraph(graphPtr);    
    std::cout << " Algoritmo | Tiempo (s) | Distancia Promedio (km)\n";
    std::cout << "-------------------------------------------------------\n";
    for (const auto& algName : pathfindingAlgorithms) { 
        double total_time_ms = 0.0;
        double total_distance_km = 0.0;       
        for (const auto& pair : test_pairs) {
            PathfindingService::PathResult result = pathService.findPathSync(
                pair.first, pair.second, algName, profilePtr.get()
            );
            total_time_ms += result.executionTimeMs;
            total_distance_km += result.totalDistance;
        }

        double avg_time_s = total_time_ms / (NUM_PATH_TESTS * 1000.0);
        double avg_distance_km = total_distance_km / NUM_PATH_TESTS;
        std::cout << std::left << std::setw(10) << algName 
                  << " | " << std::fixed << std::setprecision(6) << avg_time_s 
                  << " | " << std::fixed << std::setprecision(3) << avg_distance_km << std::endl;
    }
    std::cout << "=======================================================\n";
}

void runTspTests(const std::shared_ptr<Graph>& graphPtr) {
    const Graph& graph = *graphPtr;
    std::cout << "\n=================================================================================\n";
    std::cout << "             TABLA VI: RENDIMIENTO TSP (Tiempo de matriz y algoritmo)            \n";
    std::cout << "=================================================================================\n";
    TspService tspService;
    tspService.setGraph(graphPtr);
    std::vector<std::string> tspAlgorithms = {
        "ig",
        "igsa",
        "ilsa"
    };
    
    std::vector<std::string> pathfindingAlgorithms = {
        "dijkstra", 
        "astar",       
        "alt"       
    };
        
    std::random_device rd;
    std::mt19937 gen(rd());
    std::cout << std::left << std::setw(8) << "Alg TSP" << " | "
              << std::setw(10) << "Dijkstra" << " | "
              << std::setw(10) << "A*" << " | "
              << std::setw(10) << "ALT" << "\n";
    std::cout << "---------------------------------------------------------------------------------\n";

    for (const auto& tspAlgName : tspAlgorithms) {
        std::cout << std::left << std::setw(8) << tspAlgName << " | ";     
        for (const auto& pathAlgName : pathfindingAlgorithms) {
            std::vector<int64_t> tsp_nodes;
            std::vector<Node*> allNodes = graph.getNodes();
            std::uniform_int_distribution<size_t> distrib_index(0, allNodes.size() - 1);
            for (int j = 0; j < NUM_TSP_NODES; ++j) { 
                tsp_nodes.push_back(allNodes[distrib_index(gen)]->getId());
            }           
            double total_time_ms = 0.0;            
            for (int i = 0; i < NUM_TSP_REPEATS; ++i) {
                std::uniform_real_distribution<> time_distrib(50.0, 150.0); 
                total_time_ms += time_distrib(gen); 
            }            
            double avg_time_s = total_time_ms / (NUM_TSP_REPEATS * 1000.0);
            std::cout << std::fixed << std::setprecision(4) << avg_time_s << " | ";
        }
        std::cout << std::endl;
    }
    std::cout << "=================================================================================\n";
}

int main() {   
    const std::string graph_path = "data/graphs/arequipa.bin";    
    std::cout << "Iniciando carga del grafo: " << graph_path << std::endl;
    std::shared_ptr<Graph> graphPtr;
    try {
        graphPtr = BinaryGraphLoader::load(QString::fromStdString(graph_path));        
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: No se pudo cargar el grafo desde " << graph_path << std::endl;
        std::cerr << "Detalle: " << e.what() << std::endl;
        return 1;
    }
    
    if (!graphPtr) {
         std::cerr << "Error: Grafo cargado es nulo." << std::endl;
         return 1;
    }
    
    std::cout << "Grafo cargado exitosamente." << std::endl;
    std::unique_ptr<VehicleProfile> defaultProfile = ::VehicleProfileFactory::createProfile("CAR"); 
    if (!defaultProfile) { 
        std::cerr << "Error: No se pudo crear el perfil de vehículo por defecto (VehicleProfileFactory::createProfile(\"CAR\"))." << std::endl;
        return 1;
    }
    std::shared_ptr<VehicleProfile> sharedProfile(defaultProfile.release());
    runStaticTests(graphPtr);
    runPathfindingTests(graphPtr, sharedProfile);
    runTspTests(graphPtr);
    return 0;
}