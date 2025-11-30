#include "ControlPanel.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QMessageBox>
#include <QDebug>

namespace ui {

ControlPanel::ControlPanel(QWidget* parent)
    : QWidget(parent),
      graphLoaded_(false),
      isTspMode_(true) {  // Default: TSP
    setupUi();
}

ControlPanel::~ControlPanel() = default;

void ControlPanel::setupUi() {
    //  Layout principal del ControlPanel (solo contendrá el scrollArea)
    auto* wrapperLayout = new QVBoxLayout(this);
    wrapperLayout->setContentsMargins(0, 0, 0, 0);
    wrapperLayout->setSpacing(0);

    //  Widget contenedor para todo el contenido scrolleable
    auto* contentWidget = new QWidget();
    auto* mainLayout = new QVBoxLayout(contentWidget);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Título
    auto* titleLabel = new QLabel("<b>Panel de Control</b>", contentWidget);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(12);
    titleLabel->setFont(titleFont);
    mainLayout->addWidget(titleLabel);

    // Modo de operación
    auto* modeGroup = new QGroupBox("Modo de Operación", contentWidget);
    modeGroup->setMinimumHeight(80);
    auto* modeLayout = new QFormLayout(modeGroup);

    modeCombo_ = new QComboBox(contentWidget);
    modeCombo_->addItem("TSP (Problema del Viajante)");
    modeCombo_->addItem("Ruta Corta (Pathfinding)");
    modeCombo_->setCurrentIndex(0);
    modeCombo_->setMinimumWidth(200);
    modeCombo_->setMinimumHeight(30);
    modeCombo_->setMaximumHeight(30);
    modeLayout->addRow("Modo:", modeCombo_);

    connect(modeCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ControlPanel::onModeChanged);

    mainLayout->addWidget(modeGroup);

    // Perfil de vehículo
    auto* profileGroup = new QGroupBox("Perfil de Vehículo", contentWidget);
    profileGroup->setMinimumHeight(80);
    auto* profileLayout = new QFormLayout(profileGroup);

    profileCombo_ = new QComboBox(contentWidget);
    profileCombo_->addItem("Sin Restricciones");
    profileCombo_->addItem("Automovil");
    profileCombo_->addItem("Peaton");
    profileCombo_->setCurrentIndex(0);
    profileCombo_->setMinimumWidth(200);
    profileCombo_->setMinimumHeight(30);
    profileCombo_->setMaximumHeight(30);
    profileLayout->addRow("Perfil:", profileCombo_);

    connect(profileCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ControlPanel::onProfileChanged);

    mainLayout->addWidget(profileGroup);

    // Algoritmos
    auto* algorithmsGroup = new QGroupBox("Algoritmos", contentWidget);
    algorithmsGroup->setMinimumHeight(120);
    auto* algorithmsLayout = new QFormLayout(algorithmsGroup);

    // Algoritmo de Pathfinding (siempre visible)
    pathfindingAlgorithmCombo_ = new QComboBox(contentWidget);
    pathfindingAlgorithmCombo_->addItem("Dijkstra");
    pathfindingAlgorithmCombo_->addItem("A*");
    pathfindingAlgorithmCombo_->addItem("ALT");
    pathfindingAlgorithmCombo_->setCurrentIndex(0);
    pathfindingAlgorithmCombo_->setMinimumWidth(200);
    pathfindingAlgorithmCombo_->setMinimumHeight(30);
    pathfindingAlgorithmCombo_->setMaximumHeight(30);
    algorithmsLayout->addRow("Ruta Corta:", pathfindingAlgorithmCombo_);

    connect(pathfindingAlgorithmCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ControlPanel::onPathfindingAlgorithmChanged);

    // Algoritmo TSP (solo visible en modo TSP)
    tspAlgorithmGroup_ = new QGroupBox(contentWidget);
    tspAlgorithmGroup_->setFlat(true);
    tspAlgorithmGroup_->setStyleSheet("QGroupBox { border: 0px; }");
    auto* tspAlgLayout = new QFormLayout(tspAlgorithmGroup_);
    tspAlgLayout->setContentsMargins(0, 0, 0, 0);

    tspAlgorithmCombo_ = new QComboBox(contentWidget);
    tspAlgorithmCombo_->addItem("IG (Iterated Greedy)");
    tspAlgorithmCombo_->addItem("ILS_B (Iterated Local Search B)");
    tspAlgorithmCombo_->addItem("IGSA (IG + Simulated Annealing)");
    tspAlgorithmCombo_->setCurrentIndex(0);
    tspAlgorithmCombo_->setMinimumWidth(200);
    tspAlgorithmCombo_->setMinimumHeight(30);
    tspAlgorithmCombo_->setMaximumHeight(30);
    tspAlgLayout->addRow("TSP:", tspAlgorithmCombo_);

    connect(tspAlgorithmCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ControlPanel::onTspAlgorithmChanged);

    algorithmsLayout->addRow(tspAlgorithmGroup_);

    mainLayout->addWidget(algorithmsGroup);

    // === SELECCIÓN MANUAL ===
    manualSelectionGroup_ = new QGroupBox("Selección Manual de Nodos", contentWidget);
    manualSelectionGroup_->setMinimumHeight(180);
    auto* manualLayout = new QVBoxLayout(manualSelectionGroup_);

    auto* infoLabel = new QLabel(
        "<i>Por defecto: Selección Automática<br>"
        "Solo 1 botón puede estar activo a la vez</i>",
        contentWidget
        );
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("QLabel { font-size: 9px; color: #666; }");
    manualLayout->addWidget(infoLabel);

    manualStartButton_ = new QPushButton(" Nodo Inicio", contentWidget);
    manualStartButton_->setCheckable(true);
    manualStartButton_->setChecked(false);
    manualStartButton_->setMinimumHeight(35);
    manualStartButton_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    manualStartButton_->setStyleSheet(
        "QPushButton:checked { background-color: #0078d4; color: white; font-weight: bold; }"
        );
    manualLayout->addWidget(manualStartButton_);

    connect(manualStartButton_, &QPushButton::clicked,
            this, &ControlPanel::onManualStartButtonClicked);

    manualDestButton_ = new QPushButton(" Nodo/s Destino", contentWidget);
    manualDestButton_->setCheckable(true);
    manualDestButton_->setChecked(false);
    manualDestButton_->setMinimumHeight(35);
    manualDestButton_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    manualDestButton_->setStyleSheet(
        "QPushButton:checked { background-color: #0078d4; color: white; font-weight: bold; }"
        );
    manualLayout->addWidget(manualDestButton_);

    connect(manualDestButton_, &QPushButton::clicked,
            this, &ControlPanel::onManualDestButtonClicked);

    mainLayout->addWidget(manualSelectionGroup_);

    // === OPCIONES TSP ===
    tspOptionsGroup_ = new QGroupBox("Opciones TSP", contentWidget);
    tspOptionsGroup_->setMinimumHeight(70);
    auto* tspOptionsLayout = new QVBoxLayout(tspOptionsGroup_);

    returnToStartCheckbox_ = new QCheckBox("Volver al nodo de inicio", contentWidget);
    returnToStartCheckbox_->setChecked(true);
    tspOptionsLayout->addWidget(returnToStartCheckbox_);

    connect(returnToStartCheckbox_, &QCheckBox::toggled,
            this, &ControlPanel::onReturnToStartToggled);

    mainLayout->addWidget(tspOptionsGroup_);
    ////
    // === BOTÓN LIMPIAR SELECCIÓN ===
    clearButton_ = new QPushButton("Limpiar Selección", contentWidget);
    clearButton_->setEnabled(false);  // Deshabilitado por defecto
    clearButton_->setMinimumHeight(40);
    clearButton_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    clearButton_->setStyleSheet(
        "QPushButton { "
        "  padding: 8px; "
        "  font-size: 11px; "
        "  font-weight: bold; "
        "  background-color: #dc3545; "  // Rojo
        "  color: white; "
        "  border-radius: 5px; "
        "} "
        "QPushButton:hover { background-color: #c82333; } "
        "QPushButton:disabled { background-color: #ccc; color: #666; }"
        );
    mainLayout->addWidget(clearButton_);

    connect(clearButton_, &QPushButton::clicked,
            this, &ControlPanel::onClearButtonClicked);

    ////
    // === BOTÓN CALCULAR ===
    calculateButton_ = new QPushButton("Calcular Ruta", contentWidget);
    calculateButton_->setEnabled(false);
    calculateButton_->setMinimumHeight(45);
    calculateButton_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    calculateButton_->setStyleSheet(
        "QPushButton { "
        "  padding: 10px; "
        "  font-size: 12px; "
        "  font-weight: bold; "
        "  background-color: #28a745; "
        "  color: white; "
        "  border-radius: 5px; "
        "} "
        "QPushButton:hover { background-color: #218838; } "
        "QPushButton:disabled { background-color: #ccc; color: #666; }"
        );
    mainLayout->addWidget(calculateButton_);

    connect(calculateButton_, &QPushButton::clicked,
            this, &ControlPanel::onCalculateButtonClicked);

    //  CREAR SCROLL AREA y envolver el contentWidget
    auto* scrollArea = new QScrollArea(this);
    scrollArea->setWidget(contentWidget);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setFrameShape(QFrame::NoFrame);

    //  Agregar scrollArea al layout wrapper
    wrapperLayout->addWidget(scrollArea);

    //  Configurar el ControlPanel
    setLayout(wrapperLayout);
    setMinimumWidth(300);
    setMaximumWidth(400);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    // Configurar visibilidad inicial
    updateVisibilityForMode();
}

void ControlPanel::setGraphLoaded(bool loaded) {
    graphLoaded_ = loaded;
    calculateButton_->setEnabled(loaded);
    
    if (loaded) {
        calculateButton_->setText("Calcular Ruta");
    } else {
        calculateButton_->setText("Cargando grafo...");
    }
}

void ControlPanel::setHasSelection(bool hasSelection) {
    clearButton_->setEnabled(hasSelection);
}

void ControlPanel::onModeChanged(int index) {
    bool newIsTspMode = (index == 0);  // 0 = TSP, 1 = Ruta Corta
    
    if (newIsTspMode != isTspMode_) {
        isTspMode_ = newIsTspMode;
        
        qDebug() << "ControlPanel: Modo cambiado a"
                 << (isTspMode_ ? "TSP" : "Pathfinding");
        
        // Resetear selecciones (excepto perfil y algoritmo pathfinding)
        resetSelectionsForModeChange();
        
        // Actualizar visibilidad
        updateVisibilityForMode();
        
        // Emitir señal
        emit modeChanged(isTspMode_);
    }
}

void ControlPanel::onProfileChanged(int index) {
    QString profile;
    switch (index) {
        case 0: profile = "Sin Restricciones"; break;
        case 1: profile = "Automovil"; break;
        case 2: profile = "Peaton"; break;
        default: profile = "Sin Restricciones";
    }
    
    qDebug() << "ControlPanel: Perfil cambiado a" << profile;
    emit profileChanged(profile);
}

void ControlPanel::onPathfindingAlgorithmChanged(int index) {
    QString algorithm;
    switch (index) {
        case 0: algorithm = "dijkstra"; break;
        case 1: algorithm = "astar"; break;
        case 2: algorithm = "alt"; break;
        default: algorithm = "dijkstra";
    }
    
    qDebug() << "ControlPanel: Algoritmo pathfinding cambiado a" << algorithm;
    emit pathfindingAlgorithmChanged(algorithm);
}

void ControlPanel::onTspAlgorithmChanged(int index) {
    QString algorithm;
    switch (index) {
        case 0: algorithm = "ig"; break;
        case 1: algorithm = "ils_b"; break;
        case 2: algorithm = "igsa"; break;
        default: algorithm = "ig";
    }
    
    qDebug() << "ControlPanel: Algoritmo TSP cambiado a" << algorithm;
    emit tspAlgorithmChanged(algorithm);
}

void ControlPanel::onReturnToStartToggled(bool checked) {
    qDebug() << "ControlPanel: Volver a inicio =" << checked;
    emit returnToStartChanged(checked);
}

void ControlPanel::onManualStartButtonClicked() {
    if (manualStartButton_->isChecked()) {
        // Activar selección de inicio
        manualDestButton_->setChecked(false);  // Desactivar el otro
        qDebug() << "ControlPanel: Selección manual de INICIO activada";
        emit manualStartSelectionChanged(true);
    } else {
        // Desactivar (volver a automático)
        qDebug() << "ControlPanel: Volviendo a selección automática";
        emit manualStartSelectionChanged(false);
    }
}

void ControlPanel::onManualDestButtonClicked() {
    if (manualDestButton_->isChecked()) {
        // Activar selección de destinos
        manualStartButton_->setChecked(false);  // Desactivar el otro
        qDebug() << "ControlPanel: Selección manual de DESTINOS activada";
        emit manualDestSelectionChanged(true);
    } else {
        // Desactivar (volver a automático)
        qDebug() << "ControlPanel: Volviendo a selección automática";
        emit manualDestSelectionChanged(false);
    }
}

void ControlPanel::onCalculateButtonClicked() {
    // Por ahora solo emite señal, MainWindow manejará la lógica
    qDebug() << "ControlPanel: Botón calcular presionado";
    emit calculateRequested(std::vector<int64_t>());  // MainWindow obtendrá nodos del mapa
}

void ControlPanel::onClearButtonClicked() {  //  NUEVO
    qDebug() << "ControlPanel: Botón limpiar presionado";

    // Desactivar botones de selección manual
    deselectAllManualButtons();

    // Deshabilitar el botón limpiar hasta que haya nueva selección
    clearButton_->setEnabled(false);

    // Emitir señal para que MainWindow limpie todo
    emit clearSelectionRequested();
}

void ControlPanel::updateVisibilityForMode() {
    // Mostrar/ocultar algoritmo TSP
    tspAlgorithmGroup_->setVisible(isTspMode_);
    
    // Mostrar/ocultar opciones TSP
    tspOptionsGroup_->setVisible(isTspMode_);
    
    // Actualizar texto del botón calcular
    if (isTspMode_) {
        calculateButton_->setText("Resolver TSP");
    } else {
        calculateButton_->setText("Calcular Ruta Corta");
    }
}

void ControlPanel::resetSelectionsForModeChange() {
    // Desactivar botones de selección manual
    deselectAllManualButtons();
    
    // Resetear checkbox "Volver a inicio" a default
    returnToStartCheckbox_->setChecked(true);
    
    // NO resetear perfil ni algoritmo pathfinding (según INDICACIONES.md)
}

void ControlPanel::deselectAllManualButtons() {
    manualStartButton_->setChecked(false);
    manualDestButton_->setChecked(false);
}

} // namespace ui
