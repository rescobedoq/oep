#pragma once

#include <QWidget>
#include <QTextEdit>
#include <QTreeWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QGroupBox>
#include <vector>
#include "../services/PathfindingService.h"
#include "../services/TspService.h"

namespace ui {

/**
 * @brief Panel de resultados con detalles por tramo
 * 
 * Muestra:
 * - Distancia total
 * - Tiempo total
 * - Tramos de la ruta (Inicio → A → B → ...)
 * - Para cada tramo:
 *   - Distancia del tramo
 *   - Tiempo del tramo
 *   - Nombres de calles en orden
 */
class ResultsPanel : public QWidget {
    Q_OBJECT

public:
    explicit ResultsPanel(QWidget* parent = nullptr);
    ~ResultsPanel();

    /**
     * @brief Muestra resultados de pathfinding
     */
    void displayPathResult(const PathfindingService::PathResult& result);
    
    /**
     * @brief Muestra resultados de TSP
     */
    void displayTspResult(const TspService::TspResult& result);
    
    /**
     * @brief Limpia todos los resultados
     */
    void clear();

private:
    void setupUi();
    
    /**
     * @brief Formatea la distancia en km/m
     */
    QString formatDistance(double meters) const;
    
    /**
     * @brief Formatea el tiempo en min/seg
     */
    QString formatTime(double milliseconds) const;
    
    /**
     * @brief Calcula velocidad promedio (asumiendo 40 km/h en ciudad)
     */
    double calculateTime(double distanceMeters) const;

    // UI Components
    QLabel* titleLabel_;
    QLabel* summaryLabel_;
    QTreeWidget* detailsTree_;
    
    // Constants
    static constexpr double AVERAGE_SPEED_KMH = 40.0; // Velocidad promedio en ciudad
};

} // namespace ui
