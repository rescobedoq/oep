#include "MainWindow.h"
#include "../core/entities/Graph.h"
#include "../services/GraphService.h"
#include "../services/PathfindingService.h"
#include "../services/TspService.h"
#include "../algorithms/factories/VehicleProfileFactory.h"
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QDebug>

namespace ui {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      lastPathResult_(nullptr),
      lastTspResult_(nullptr),
      isTspMode_(true),  // Default: TSP
      currentProfile_("Sin Restricciones"),
      currentPathfindingAlgorithm_("dijkstra"),
      currentTspAlgorithm_("ig"),
      returnToStart_(true) {
    
    // Crear servicios
    graphService_ = new services::GraphService();
    pathfindingService_ = new PathfindingService();
    tspService_ = new TspService();
    
    setupUi();
    setupMenuBar();
    connectSignals();
    
    // Cargar grafo por defecto
    loadDefaultGraph();
}

MainWindow::~MainWindow() {
    delete graphService_;
    delete pathfindingService_;
    delete tspService_;
    
    if (lastPathResult_) {
        delete static_cast<PathfindingService::PathResult*>(lastPathResult_);
        lastPathResult_ = nullptr;
    }
    if (lastTspResult_) {
        delete static_cast<TspService::TspResult*>(lastTspResult_);
        lastTspResult_ = nullptr;
    }
}

void MainWindow::setupUi() {
    setWindowTitle("Routing App - C++ / Qt6");
    resize(1600, 1000);
    showMaximized();

    // Crear widgets principales
    mapWidget_ = new MapWidget(this);
    controlPanel_ = new ControlPanel(this);
    controlPanel_->setFixedWidth(350);
    resultsPanel_ = new ResultsPanel(this);
    
    // Layout horizontal: ControlPanel | MapWidget
    mainSplitter_ = new QSplitter(Qt::Horizontal, this);
    mainSplitter_->addWidget(controlPanel_);
    mainSplitter_->addWidget(mapWidget_);
    mainSplitter_->setStretchFactor(0, 0);  // Control no se estira
    mainSplitter_->setStretchFactor(1, 1);  // Map si se estira
    mainSplitter_->setSizes({320, 1280});   // Tamaños iniciales
    mainSplitter_->setCollapsible(0, false);  // ControlPanel no puede colapsar

    // Layout vertical: mainSplitter arriba, resultsPanel abajo
    verticalSplitter_ = new QSplitter(Qt::Vertical, this);
    verticalSplitter_->addWidget(mainSplitter_);
    verticalSplitter_->addWidget(resultsPanel_);
    verticalSplitter_->setStretchFactor(0, 3);  // Map 75%
    verticalSplitter_->setStretchFactor(1, 1);  // Results 25%
    resultsPanel_->setMinimumHeight(150);
    setCentralWidget(verticalSplitter_);
    
    // Status bar
    statusBar()->showMessage("Listo");
}

void MainWindow::setupMenuBar() {
    QMenu* fileMenu = menuBar()->addMenu("&Archivo");
    
    QAction* loadAction = fileMenu->addAction("&Cargar Grafo...");
    connect(loadAction, &QAction::triggered, [this]() {
        loadDefaultGraph();
    });
    
    fileMenu->addSeparator();
    
    QAction* exitAction = fileMenu->addAction("&Salir");
    connect(exitAction, &QAction::triggered, this, &QMainWindow::close);
    
    QMenu* helpMenu = menuBar()->addMenu("&Ayuda");
    
    QAction* aboutAction = helpMenu->addAction("&Acerca de...");
    connect(aboutAction, &QAction::triggered, [this]() {
        QMessageBox::about(this, "Acerca de Routing App",
            "Routing App - C++ / Qt6\n\n"
            "Aplicación de ruteo con algoritmos de pathfinding y TSP.\n"
            "Migrado desde Java/JavaFX a C++/Qt6.\n\n"
            "Universidad Nacional de San Agustín\n"
            "© 2025");
    });
}

void MainWindow::connectSignals() {
    // === GraphService signals ===
    connect(graphService_, &services::GraphService::graphLoaded,
            this, &MainWindow::onGraphLoaded);
    
    connect(graphService_, &services::GraphService::loadError,
            this, &MainWindow::onGraphLoadError);
    
    connect(graphService_, &services::GraphService::loadProgress,
            this, &MainWindow::onGraphLoadProgress);
    
    // === ControlPanel signals ===
    connect(controlPanel_, &ControlPanel::modeChanged,
            this, &MainWindow::onModeChanged);

    connect(controlPanel_, &ControlPanel::profileChanged,
            this, &MainWindow::onProfileChanged);

    connect(controlPanel_, &ControlPanel::pathfindingAlgorithmChanged,
            this, &MainWindow::onPathfindingAlgorithmChanged);

    connect(controlPanel_, &ControlPanel::tspAlgorithmChanged,
            this, &MainWindow::onTspAlgorithmChanged);

    connect(controlPanel_, &ControlPanel::returnToStartChanged,
            this, &MainWindow::onReturnToStartChanged);

    connect(controlPanel_, &ControlPanel::manualStartSelectionChanged,
            this, &MainWindow::onManualStartSelectionChanged);

    connect(controlPanel_, &ControlPanel::manualDestSelectionChanged,
            this, &MainWindow::onManualDestSelectionChanged);

    connect(controlPanel_, &ControlPanel::calculateRequested,
            this, &MainWindow::onCalculateRequested);

    connect(controlPanel_, &ControlPanel::clearSelectionRequested,
            this, &MainWindow::onClearSelectionRequested);

    // === MapWidget signals ===
    connect(mapWidget_, &MapWidget::nodeSelected,
            this, &MainWindow::onMapNodeSelected);
    
    connect(mapWidget_, &MapWidget::selectionChanged,
            this, &MainWindow::onMapSelectionChanged);
    
    // === PathfindingService signals ===
    connect(pathfindingService_, &PathfindingService::pathFound,
            this, [this](const PathfindingService::PathResult& result) {
                // Store result
                delete static_cast<PathfindingService::PathResult*>(lastPathResult_);
                lastPathResult_ = new PathfindingService::PathResult(result);
                onPathFound();
            });

    connect(pathfindingService_, &PathfindingService::pathError,
            this, &MainWindow::onPathError);

    // === TspService signals ===
    connect(tspService_, &TspService::tspSolved,
            this, [this](const TspService::TspResult& result) {
                // Store result
                delete static_cast<TspService::TspResult*>(lastTspResult_);
                lastTspResult_ = new TspService::TspResult(result);
                onTspSolved();
            });

    connect(tspService_, &TspService::tspError,
            this, &MainWindow::onTspError);

}

void MainWindow::loadDefaultGraph() {
    statusBar()->showMessage("Cargando grafo de Arequipa...");
    graphService_->loadGraphAsync("arequipa");
}

// === GraphService slots ===
void MainWindow::onGraphLoaded(std::shared_ptr<Graph> graph, qint64 loadTimeMs) {
    currentGraph_ = graph;
    
    qDebug() << "   Grafo cargado en MainWindow:";
    qDebug() << "   Tiempo:" << loadTimeMs << "ms";
    qDebug() << "   Nodos:" << graph->getNodeCount();
    qDebug() << "   Aristas:" << graph->getEdgeCount();
    
    // Actualizar UI
    mapWidget_->setGraph(graph);
    controlPanel_->setGraphLoaded(true);
    
    // Actualizar servicios
    pathfindingService_->setGraph(graph);
    tspService_->setGraph(graph);
    
    statusBar()->showMessage(
        QString("Grafo cargado: %1 nodos, %2 aristas (%3 ms)")
            .arg(graph->getNodeCount())
            .arg(graph->getEdgeCount())
            .arg(loadTimeMs),
        5000
    );
    
    QMessageBox::information(this, "Grafo Cargado",
        QString("Grafo cargado exitosamente en %1 ms\n\n"
                "Nodos: %2\n"
                "Aristas: %3")
            .arg(loadTimeMs)
            .arg(graph->getNodeCount())
            .arg(graph->getEdgeCount())
    );
}

void MainWindow::onGraphLoadError(const QString& errorMessage) {
    qDebug() << " Error cargando grafo:" << errorMessage;
    
    statusBar()->showMessage("Error: " + errorMessage, 10000);
    
    QMessageBox::critical(this, "Error de Carga",
        QString("No se pudo cargar el grafo:\n\n%1").arg(errorMessage));
}

void MainWindow::onGraphLoadProgress(const QString& message, double progress) {
    if (progress < 0) {
        statusBar()->showMessage(message);
    } else {
        statusBar()->showMessage(QString("%1 (%2%)")
            .arg(message)
            .arg(static_cast<int>(progress * 100)));
    }
}

// === ControlPanel slots ===

void MainWindow::onModeChanged(bool isTsp) {
    isTspMode_ = isTsp;
    qDebug() << "MainWindow: Modo cambiado a" << (isTsp ? "TSP" : "Pathfinding");

    // Actualizar modo en mapa
    mapWidget_->setOperationMode(isTsp ? OperationMode::TSP : OperationMode::PATHFINDING);

    // Limpiar selección y resultados
    mapWidget_->clearSelection();
    mapWidget_->clearPath();
    resultsPanel_->clear();
}

void MainWindow::onProfileChanged(const QString& profile) {
    currentProfile_ = profile;
    qDebug() << "MainWindow: Perfil cambiado a" << profile;

    // Actualizar perfil en mapa (redibujar con bloqueos)
    mapWidget_->setVehicleProfile(profile);
}

void MainWindow::onPathfindingAlgorithmChanged(const QString& algorithm) {
    currentPathfindingAlgorithm_ = algorithm;
    qDebug() << "MainWindow: Algoritmo pathfinding cambiado a" << algorithm;
}

void MainWindow::onTspAlgorithmChanged(const QString& algorithm) {
    currentTspAlgorithm_ = algorithm;
    qDebug() << "MainWindow: Algoritmo TSP cambiado a" << algorithm;
}

void MainWindow::onReturnToStartChanged(bool returnToStart) {
    returnToStart_ = returnToStart;
    qDebug() << "MainWindow: Volver a inicio =" << returnToStart;
}

void MainWindow::onManualStartSelectionChanged(bool active) {
    if (active) {
        mapWidget_->setSelectionMode(SelectionMode::MANUAL_START);
    } else {
        mapWidget_->setSelectionMode(SelectionMode::AUTOMATIC);
    }
}

void MainWindow::onManualDestSelectionChanged(bool active) {
    if (active) {
        mapWidget_->setSelectionMode(SelectionMode::MANUAL_DEST);
    } else {
        mapWidget_->setSelectionMode(SelectionMode::AUTOMATIC);
    }
}

void MainWindow::onCalculateRequested(const std::vector<int64_t>& nodeIds) {
    // Obtener nodos seleccionados del mapa
    std::vector<int64_t> selectedNodes = mapWidget_->getSelectedNodes();

    if (!validateNodeSelection(selectedNodes)) {
        return;
    }

    if (isTspMode_) {
        // Resolver TSP
        int64_t startNode = selectedNodes[0];

        qDebug() << "   Resolviendo TSP:";
        qDebug() << "   Nodos:" << selectedNodes.size();
        qDebug() << "   Inicio:" << startNode;
        qDebug() << "   Algoritmo:" << currentTspAlgorithm_;
        qDebug() << "   Retornar:" << returnToStart_;

        statusBar()->showMessage("Resolviendo TSP...");

        // Crear VehicleProfile según perfil seleccionado
        std::unique_ptr<VehicleProfile> vehicleProfile = nullptr;
        if (currentProfile_ == "Automovil") {
            vehicleProfile = VehicleProfileFactory::createCarProfile();
        } else if (currentProfile_ == "Peaton") {
            vehicleProfile = VehicleProfileFactory::createPedestrianProfile();
        }

        tspService_->solveAsync(
            selectedNodes,
            currentTspAlgorithm_.toStdString(),
            currentPathfindingAlgorithm_.toStdString(),
            vehicleProfile.get(),  // Pasar el perfil correcto
            returnToStart_
            );
    } else {
        // Pathfinding
        int64_t startNode = selectedNodes[0];
        int64_t endNode = selectedNodes[1];

        qDebug() << "   Buscando ruta:";
        qDebug() << "   Inicio:" << startNode;
        qDebug() << "   Fin:" << endNode;
        qDebug() << "   Algoritmo:" << currentPathfindingAlgorithm_;
        qDebug() << "   Perfil:" << currentProfile_;

        statusBar()->showMessage("Buscando ruta...");

        // Crear VehicleProfile según perfil seleccionado
        std::unique_ptr<VehicleProfile> vehicleProfile = nullptr;
        if (currentProfile_ == "Automovil") {
            vehicleProfile = VehicleProfileFactory::createCarProfile();
        } else if (currentProfile_ == "Peaton") {
            vehicleProfile = VehicleProfileFactory::createPedestrianProfile();
        }

        pathfindingService_->findPathAsync(
            startNode,
            endNode,
            currentPathfindingAlgorithm_.toStdString(),
            vehicleProfile.get()  // Pasar el perfil correcto
            );
    }
}

bool MainWindow::validateNodeSelection(const std::vector<int64_t>& nodeIds) const {
    if (!currentGraph_) {
        QMessageBox::warning(const_cast<MainWindow*>(this), "Error",
                             "No hay grafo cargado");
        return false;
    }

    if (isTspMode_) {
        // TSP: mínimo 2 nodos (inicio + 1 destino)
        if (nodeIds.size() < 2) {
            QMessageBox::warning(const_cast<MainWindow*>(this), "Selección Insuficiente",
                                 "Por favor seleccione al menos 2 nodos:\n"
                                 "- 1 nodo de inicio (verde)\n"
                                 "- 1 o más nodos destino (naranjas con letras)");
            return false;
        }
    } else {
        // Pathfinding: exactamente 2 nodos
        if (nodeIds.size() != 2) {
            QMessageBox::warning(const_cast<MainWindow*>(this), "Selección Incorrecta",
                                 "Por favor seleccione exactamente 2 nodos:\n"
                                 "- Nodo de inicio\n"
                                 "- Nodo de destino\n\n"
                                 "Límite: 2 nodos máximo para Ruta Corta");
            return false;
        }
    }

    return true;
}

// === MapWidget slots ===
void MainWindow::onMapNodeSelected(int64_t nodeId, bool isSelected) {
    if (isSelected) {
        qDebug() << " MainWindow: Nodo" << nodeId << "seleccionado";
    } else {
        qDebug() << " MainWindow: Nodo" << nodeId << "deseleccionado";
    }
}

void MainWindow::onMapSelectionChanged(const std::vector<int64_t>& selectedNodes) {
    qDebug() << " MainWindow: Selección cambiada:" << selectedNodes.size() << "nodos";
    // Habilitar/deshabilitar botón limpiar
    controlPanel_->setHasSelection(!selectedNodes.empty());

    // Actualizar status bar
    if (selectedNodes.empty()) {
        statusBar()->showMessage("Seleccione nodos en el mapa", 3000);
    } else {
        QString message = QString("Nodos seleccionados: %1").arg(selectedNodes.size());
        
        if (isTspMode_) {
            message += " (TSP: inicio + destinos)";
        } else {
            message += QString(" (Pathfinding: %1/2)").arg(selectedNodes.size());
        }
        
        statusBar()->showMessage(message, 3000);
    }
}

void MainWindow::onClearSelectionRequested() {
    qDebug() << "MainWindow: Limpiando selección y resultados";

    // Limpiar selección de nodos en el mapa
    mapWidget_->clearSelection();

    // Limpiar rutas visualizadas
    mapWidget_->clearPath();

    // Limpiar panel de resultados
    resultsPanel_->clear();

    // Actualizar status bar
    statusBar()->showMessage("Selección limpiada", 3000);
}

// === PathfindingService slots ===

void MainWindow::onPathFound() {
    if (!lastPathResult_) return;

    auto& result = *static_cast<PathfindingService::PathResult*>(lastPathResult_);

    qDebug() << "   Ruta encontrada:";
    qDebug() << "   Distancia:" << result.totalDistance << "m";
    qDebug() << "   Tiempo:" << result.executionTimeMs << "ms";
    qDebug() << "   Aristas:" << result.pathEdges.size();

    // Actualizar mapa
    mapWidget_->displayPath(result.pathEdges);

    // Actualizar panel de resultados
    resultsPanel_->displayPathResult(result);

    statusBar()->showMessage(
        QString("Ruta encontrada: %1 m en %2 ms")
            .arg(result.totalDistance, 0, 'f', 1)
            .arg(result.executionTimeMs),
        5000
        );
}

void MainWindow::onPathError(const QString& error) {
    qDebug() << "Error en pathfinding:" << error;

    QMessageBox::warning(this, "Error en Pathfinding", error);
    statusBar()->showMessage("Error: " + error, 5000);
}

// === TspService slots ===

void MainWindow::onTspSolved() {
    if (!lastTspResult_) return;

    auto& result = *static_cast<TspService::TspResult*>(lastTspResult_);

    qDebug() << "   TSP resuelto:";
    qDebug() << "   Distancia total:" << result.totalDistance << "m";
    qDebug() << "   Orden de visita:" << result.tour.size() << "nodos";
    qDebug() << "   Segmentos:" << result.segmentEdges.size();

    // Actualizar mapa (mostrar todo el tour, sin resaltar segmento específico)
    mapWidget_->displayTspTour(result.segmentEdges, -1);

    // Actualizar panel de resultados
    resultsPanel_->displayTspResult(result);

    statusBar()->showMessage(
        QString("TSP resuelto: %1 m, %2 segmentos")
            .arg(result.totalDistance, 0, 'f', 1)
            .arg(result.segmentEdges.size()),
        5000
        );
}

void MainWindow::onTspError(const QString& error) {
    qDebug() << "Error en TSP:" << error;

    QMessageBox::warning(this, "Error en TSP", error);
    statusBar()->showMessage("Error: " + error, 5000);
}
} //namespace ui
