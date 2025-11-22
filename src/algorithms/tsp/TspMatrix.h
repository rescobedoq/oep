// src/algorithms/tsp/TspMatrix.h
#pragma once

#include "../../core/entities/Graph.h"
#include "../../core/interfaces/IPathfindingAlgorithm.h"
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <memory>
#include <functional>

/**
 * @brief Matriz TSP con distancias y paths precalculados
 * 
 * MAPEO COMPLETO de model.tsp.TspMatrix (Java)
 * - Precompute paralelo con cache bidireccional
 * - nearestNeighborRoute() para inicialización heurística
 * - getEntry() para acceder a distancias/paths
 */
class TspMatrix {
public:
    /**
     * @brief Entrada de matriz (distance + path)
     */
    struct Entry {
        double distance;
        std::vector<int64_t> pathEdgeIds;
        
        Entry() : distance(0.0) {}
        Entry(double dist, const std::vector<int64_t>& path)
            : distance(dist), pathEdgeIds(path) {}
    };
    
    /**
     * @brief Callback de progreso para UI
     * 
     * @param current Pares completados
     * @param total Pares totales
     * @param percent Porcentaje (0-100)
     */
    using ProgressCallback = std::function<void(int current, int total, int percent)>;
    
private:
    size_t size_;
    std::vector<int64_t> nodeIds_;
    
    // Matriz: matrix_[fromIdx][toIdx] = Entry
    std::vector<std::vector<Entry>> matrix_;
    
public:
    /**
     * @brief Constructor
     * 
     * @param size Número de waypoints
     * @param nodeIds IDs de nodos a visitar
     */
    TspMatrix(size_t size, const std::vector<int64_t>& nodeIds);
    
    /**
     * @brief Precomputa matriz usando pathfinding
     * 
     * MAPEO de TspMatrix constructor en Java (con ExecutorService)
     * 
     * @param graph Grafo
     * @param algorithm Algoritmo de pathfinding (Dijkstra, A*, etc.)
     * @param vehicleProfile Perfil de vehículo (puede ser nullptr)
     * @param progressCallback Callback para feedback de progreso (opcional)
     * 
     * NOTA: En C++ usamos std::thread básico (NO ThreadPool como Java)
     * Para UI responsiva, llamar desde std::thread en TspService
     */
    void precompute(
        const Graph& graph,
        IPathfindingAlgorithm* algorithm,
        const VehicleProfile* vehicleProfile = nullptr,
        ProgressCallback progressCallback = nullptr
    );
    
    /**
     * @brief Obtiene entrada de matriz
     */
    const Entry& getEntry(size_t fromIdx, size_t toIdx) const {
        return matrix_[fromIdx][toIdx];
    }
    
    /**
     * @brief Obtiene entrada de matriz por node IDs
     */
    const Entry& getEntry(int64_t fromNodeId, int64_t toNodeId) const;
    
    /**
     * @brief Establece distancia manualmente (para tests)
     */
    void setDistance(size_t fromIdx, size_t toIdx, double distance) {
        matrix_[fromIdx][toIdx].distance = distance;
    }
    
    /**
     * @brief Obtiene node ID por índice
     */
    int64_t getNodeId(size_t idx) const {
        return nodeIds_[idx];
    }
    
    /**
     * @brief Obtiene índice por node ID
     */
    size_t getNodeIndex(int64_t nodeId) const;
    
    /**
     * @brief Tamaño de matriz
     */
    size_t getSize() const {
        return size_;
    }
    
    /**
     * @brief Calcula costo de tour
     * 
     * @param tour Índices del tour (0 a N-1)
     * @param returnToStart Si debe volver al inicio
     * @return Distancia total
     */
    double calculateTourCost(const std::vector<int>& tour, bool returnToStart = false) const;
    
    /**
     * @brief Inicialización heurística Nearest Neighbor
     * 
     * MAPEO: TspMatrix.nearestNeighborRoute() en Java
     * 
     * @param startIdx Índice de inicio (0 a N-1)
     * @return Tour heurístico (índices)
     */
    std::vector<int> nearestNeighborRoute(int startIdx = 0) const;
    
    /**
     * @brief Valida que la matriz no tenga distancias infinitas
     * 
     * @return Vector de pares (fromIdx, toIdx) con distancias infinitas
     */
    std::vector<std::pair<size_t, size_t>> getUnreachablePairs() const;
    
    /**
     * @brief Verifica si la matriz tiene solución válida
     * 
     * @return true si todos los nodos son alcanzables entre sí
     */
    bool hasValidSolution() const;
    
private:
    /**
     * @brief Convierte path de edges a distancia total
     */
    double calculatePathDistance(const Graph& graph, const std::vector<int64_t>& edgeIds) const;
};
