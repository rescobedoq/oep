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
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    
    // Título
    auto* titleLabel = new QLabel("<b>Panel de Control</b>", this);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(12);
    titleLabel->setFont(titleFont);
    mainLayout->addWidget(titleLabel);
    
    // Modo de operación
    auto* modeGroup = new QGroupBox("Modo de Operación", this);
    auto* modeLayout = new QFormLayout(modeGroup);
    
    modeCombo_ = new QComboBox(this);
    modeCombo_->addItem("TSP (Problema del Viajante)");
    modeCombo_->addItem("Ruta Corta (Pathfinding)");
    modeCombo_->setCurrentIndex(0);  // Default: TSP
    modeLayout->addRow("Modo:", modeCombo_);
    
    connect(modeCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ControlPanel::onModeChanged);
    
    mainLayout->addWidget(modeGroup);
    
    // Perfil de vehículo
    auto* profileGroup = new QGroupBox("Perfil de Vehículo", this);
    auto* profileLayout = new QFormLayout(profileGroup);
    
    profileCombo_ = new QComboBox(this);
    profileCombo_->addItem("Sin Restricciones");
    profileCombo_->addItem("Automovil");
    profileCombo_->addItem("Peaton");
    profileCombo_->setCurrentIndex(0);  // Default: Sin Restricciones
    profileLayout->addRow("Perfil:", profileCombo_);
    
    connect(profileCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ControlPanel::onProfileChanged);
    
    mainLayout->addWidget(profileGroup);
    
    // Algoritmos
    auto* algorithmsGroup = new QGroupBox("Algoritmos", this);
    auto* algorithmsLayout = new QFormLayout(algorithmsGroup);
    
    // Algoritmo de Pathfinding (siempre visible)
    pathfindingAlgorithmCombo_ = new QComboBox(this);
    pathfindingAlgorithmCombo_->addItem("Dijkstra");
    pathfindingAlgorithmCombo_->addItem("A*");
    pathfindingAlgorithmCombo_->addItem("ALT");
    pathfindingAlgorithmCombo_->setCurrentIndex(0);  // Default: Dijkstra
    algorithmsLayout->addRow("Ruta Corta:", pathfindingAlgorithmCombo_);
    
    connect(pathfindingAlgorithmCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ControlPanel::onPathfindingAlgorithmChanged);
    
    // Algoritmo TSP (solo visible en modo TSP)
    tspAlgorithmGroup_ = new QGroupBox(this);
    tspAlgorithmGroup_->setFlat(true);
    tspAlgorithmGroup_->setStyleSheet("QGroupBox { border: 0px; }");
    auto* tspAlgLayout = new QFormLayout(tspAlgorithmGroup_);
    tspAlgLayout->setContentsMargins(0, 0, 0, 0);
    
    tspAlgorithmCombo_ = new QComboBox(this);
    tspAlgorithmCombo_->addItem("IG (Iterated Greedy)");
    tspAlgorithmCombo_->addItem("ILS_B (Iterated Local Search B)");
    tspAlgorithmCombo_->addItem("IGSA (IG + Simulated Annealing)");
    tspAlgorithmCombo_->setCurrentIndex(0);  // Default: IG
    tspAlgLayout->addRow("TSP:", tspAlgorithmCombo_);
    
    connect(tspAlgorithmCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ControlPanel::onTspAlgorithmChanged);
    
    algorithmsLayout->addRow(tspAlgorithmGroup_);
    
    mainLayout->addWidget(algorithmsGroup);
    
    // === SELECCIÓN MANUAL ===
    manualSelectionGroup_ = new QGroupBox("Selección Manual de Nodos", this);
    auto* manualLayout = new QVBoxLayout(manualSelectionGroup_);
    
    auto* infoLabel = new QLabel(
        "<i>Por defecto: Selección Automática<br>"
        "Solo 1 botón puede estar activo a la vez</i>",
        this
    );
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("QLabel { font-size: 9px; color: #666; }");
    manualLayout->addWidget(infoLabel);
    
    manualStartButton_ = new QPushButton("Nodo Inicio", this);
    manualStartButton_->setCheckable(true);
    manualStartButton_->setChecked(false);
    manualStartButton_->setStyleSheet(
        "QPushButton:checked { background-color: #0078d4; color: white; font-weight: bold; }"
    );
    manualLayout->addWidget(manualStartButton_);
    
    connect(manualStartButton_, &QPushButton::clicked,
            this, &ControlPanel::onManualStartButtonClicked);
    
    manualDestButton_ = new QPushButton("Nodo/s Destino", this);
    manualDestButton_->setCheckable(true);
    manualDestButton_->setChecked(false);
    manualDestButton_->setStyleSheet(
        "QPushButton:checked { background-color: #0078d4; color: white; font-weight: bold; }"
    );
    manualLayout->addWidget(manualDestButton_);
    
    connect(manualDestButton_, &QPushButton::clicked,
            this, &ControlPanel::onManualDestButtonClicked);
    
    mainLayout->addWidget(manualSelectionGroup_);
    
    // === OPCIONES TSP ===
    tspOptionsGroup_ = new QGroupBox("Opciones TSP", this);
    auto* tspOptionsLayout = new QVBoxLayout(tspOptionsGroup_);
    
    returnToStartCheckbox_ = new QCheckBox("Volver al nodo de inicio", this);
    returnToStartCheckbox_->setChecked(true);  // Default: true
    tspOptionsLayout->addWidget(returnToStartCheckbox_);
    
    connect(returnToStartCheckbox_, &QCheckBox::toggled,
            this, &ControlPanel::onReturnToStartToggled);
    
    mainLayout->addWidget(tspOptionsGroup_);
    
    // === BOTÓN CALCULAR ===
    calculateButton_ = new QPushButton("Calcular Ruta", this);
    calculateButton_->setEnabled(false);
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
    
    // === ESPACIADOR ===
    mainLayout->addStretch();
    
    setLayout(mainLayout);
    setMaximumWidth(350);
    
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
