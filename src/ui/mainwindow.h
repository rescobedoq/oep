#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QSplitter>
#include <memory>

#include "core/entities/Graph.h"
// #include "services/GraphService.h"
// #include "services/PathfindingService.h"
// #include "services/TspService.h"
#include "MapWidget.h"
// #include "ui/ControlPanel.h"
// #include "ui/ResultsPanel.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    /**
     * @brief Slot cuando el grafo se carga exitosamente
     */
    // void onGraphLoaded(std::shared_ptr<core::entities::Graph> graph, qint64 loadTimeMs);

    /**
     * @brief Slot cuando hay error en carga
     */
    //void onGraphLoadError(const QString& errorMessage);

    /**
     * @brief Slot cuando usuario solicita búsqueda de ruta
     */
    /*void onFindPathRequested(
        int64_t startNodeId,
        int64_t endNodeId,
        const QString& algorithmName,
        const QString& vehicleType
        );*/

    /**
     * @brief Slot cuando usuario solicita resolver TSP
     */
    /*void onSolveTspRequested(
        const std::vector<int64_t>& nodeIds,
        int64_t startNodeId,
        const QString& algorithmName,
        bool returnToStart
        );*/

    /**
     * @brief Slot cuando pathfinding se completa
     */
    // void onPathFound(const services::PathfindingService::PathResult& result);

    /**
     * @brief Slot cuando TSP se resuelve
     */
    //void onTspSolved(const std::vector<int>& tour, double totalDistance);

private:
    void setupUi();
    void setupMenuBar();
    void connectSignals();
    void loadDefaultGraph();

    // Servicios
    // std::unique_ptr<services::GraphService> graphService_;
    // std::unique_ptr<services::PathfindingService> pathfindingService_;
    // std::unique_ptr<services::TspService> tspService_;

    // Widgets
    //MapWidget* mapWidget_;
    // ControlPanel* controlPanel_;
    // ResultsPanel* resultsPanel_;
    QSplitter* mainSplitter_;
    QSplitter* verticalSplitter_;

    // Estado
    // std::shared_ptr<core::entities::Graph> currentGraph_;

    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
