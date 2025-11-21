#include "mainwindow.h"
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // Crear servicios
    // graphService_ = std::make_unique<services::GraphService>();
    // pathfindingService_ = std::make_unique<services::PathfindingService>();
    // tspService_ = std::make_unique<services::TspService>();

    setupUi();
    //setupMenuBar();
    //connectSignals();

    // Cargar grafo por defecto
    // loadDefaultGraph();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUi() {
    setWindowTitle("Routing App - C++ / Qt6");
    resize(1400, 900);

    // Crear widgets principales
    mapWidget_ = new MapWidget(this);
    //controlPanel_ = new ControlPanel(this);
    //resultsPanel_ = new ResultsPanel(this);

    // Layout horizontal: ControlPanel | MapWidget
    mainSplitter_ = new QSplitter(Qt::Horizontal, this);
    //mainSplitter_->addWidget(controlPanel_);
    mainSplitter_->addWidget(mapWidget_);
    mainSplitter_->setStretchFactor(0, 1); // Control 20%
    mainSplitter_->setStretchFactor(1, 4); // Map 80%

    // Layout vertical: mainSplitter arriba, resultsPanel abajo
    verticalSplitter_ = new QSplitter(Qt::Vertical, this);
    //verticalSplitter_->addWidget(mainSplitter_);
    //verticalSplitter_->addWidget(resultsPanel_);
    verticalSplitter_->setStretchFactor(0, 3); // Map 75%
    verticalSplitter_->setStretchFactor(1, 1); // Results 25%

    setCentralWidget(verticalSplitter_);

    // Status bar
    statusBar()->showMessage("Listo");
}