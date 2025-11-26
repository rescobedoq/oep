#include "MainWindow.h"
#include "src/core/entities/Graph.h"
#include "src/services/GraphService.h"
#include "src/services/PathfindingService.h"
#include "src/services/TspService.h"
#include "src/algorithms/factories/VehicleProfileFactory.h"
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QDebug>


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
    
    // Crear widgets principales
    mapWidget_ = new MapWidget(this);
    // controlPanel_ = new ControlPanel(this);
    // resultsPanel_ = new ResultsPanel(this);
    
    // Layout horizontal: ControlPanel | MapWidget
    mainSplitter_ = new QSplitter(Qt::Horizontal, this);
    // mainSplitter_->addWidget(controlPanel_);
    mainSplitter_->addWidget(mapWidget_);
    mainSplitter_->setStretchFactor(0, 1);  // Control 25%
    mainSplitter_->setStretchFactor(1, 3);  // Map 75%
    
    // Layout vertical: mainSplitter arriba, resultsPanel abajo
    verticalSplitter_ = new QSplitter(Qt::Vertical, this);
    verticalSplitter_->addWidget(mainSplitter_);
    // verticalSplitter_->addWidget(resultsPanel_);
    verticalSplitter_->setStretchFactor(0, 3);  // Map 75%
    verticalSplitter_->setStretchFactor(1, 1);  // Results 25%
    
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
    
    // === MapWidget signals ===
    connect(mapWidget_, &MapWidget::nodeSelected,
            this, &MainWindow::onMapNodeSelected);
    
    connect(mapWidget_, &MapWidget::selectionChanged,
            this, &MainWindow::onMapSelectionChanged);
    
}

void MainWindow::loadDefaultGraph() {
    statusBar()->showMessage("Cargando grafo de Arequipa...");
    graphService_->loadGraphAsync("arequipa");
}

// === GraphService slots ===
void MainWindow::onGraphLoaded(std::shared_ptr<Graph> graph, qint64 loadTimeMs) {
    currentGraph_ = graph;
    
    qDebug() << "✅ Grafo cargado en MainWindow:";
    qDebug() << "   Tiempo:" << loadTimeMs << "ms";
    qDebug() << "   Nodos:" << graph->getNodeCount();
    qDebug() << "   Aristas:" << graph->getEdgeCount();
    
    // Actualizar UI
    mapWidget_->setGraph(graph);
    //controlPanel_->setGraphLoaded(true);
    
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
    qDebug() << "❌ Error cargando grafo:" << errorMessage;
    
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

// === MapWidget slots ===
void MainWindow::onMapNodeSelected(int64_t nodeId, bool isSelected) {
    if (isSelected) {
        qDebug() << "✅ MainWindow: Nodo" << nodeId << "seleccionado";
    } else {
        qDebug() << "❌ MainWindow: Nodo" << nodeId << "deseleccionado";
    }
}

void MainWindow::onMapSelectionChanged(const std::vector<int64_t>& selectedNodes) {
    qDebug() << "🔄 MainWindow: Selección cambiada:" << selectedNodes.size() << "nodos";
    
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