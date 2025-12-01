#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include <QStatusBar>
#include <memory>
#include <vector>
#include <cstdint>
#include "../core/entities/Graph.h"
#include "MapWidget.h"
#include "ControlPanel.h"
#include "ResultsPanel.h"

class Graph; 
class PathfindingService; 
class TspService;

namespace services {
class GraphService;
}

namespace ui {
class MapWidget;
class ControlPanel;
class ResultsPanel;
}

namespace ui {


// Usar clases del namespace ui
using ui::MapWidget;
using ui::ControlPanel;
using ui::ResultsPanel;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    // GraphService signals
    void onGraphLoaded(std::shared_ptr<Graph> graph, qint64 loadTimeMs);
    void onGraphLoadError(const QString& errorMessage);
    void onGraphLoadProgress(const QString& message, double progress);

    // === ControlPanel signals ===
    void onModeChanged(bool isTsp);
    void onProfileChanged(const QString& profile);
    void onPathfindingAlgorithmChanged(const QString& algorithm);
    void onTspAlgorithmChanged(const QString& algorithm);
    void onReturnToStartChanged(bool returnToStart);
    void onManualStartSelectionChanged(bool active);
    void onManualDestSelectionChanged(bool active);
    void onCalculateRequested(const std::vector<int64_t>& nodeIds);

    // MapWidget signals
    void onMapNodeSelected(int64_t nodeId, bool isSelected);
    void onMapSelectionChanged(const std::vector<int64_t>& selectedNodes);
    void onClearSelectionRequested();

    // === PathfindingService signals ===
    void onPathFound();
    void onPathError(const QString& error);

    // === TspService signals ===
    void onTspSolved();
    void onTspError(const QString& error);

private:
    void setupUi();
    void setupMenuBar();
    void connectSignals();
    void loadDefaultGraph();

    /**
     * @brief Valida que haya nodos suficientes según el modo actual
     */
    bool validateNodeSelection(const std::vector<int64_t>& nodeIds) const;

    // SERVICIOS
    services::GraphService* graphService_;
    PathfindingService* pathfindingService_;
    TspService* tspService_;

    // WIDGETS
    MapWidget* mapWidget_;
    ControlPanel* controlPanel_;
    ResultsPanel* resultsPanel_;
    QSplitter* mainSplitter_;
    QSplitter* verticalSplitter_;

    // ESTADO
    std::shared_ptr<Graph> currentGraph_;
    
    // Últimos resultados
    void* lastPathResult_; 
    void* lastTspResult_;
    
    // Configuración actual
    bool isTspMode_;
    QString currentProfile_;
    QString currentPathfindingAlgorithm_;
    QString currentTspAlgorithm_;
    bool returnToStart_;

    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
}
