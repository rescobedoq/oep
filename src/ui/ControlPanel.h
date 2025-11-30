#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QGroupBox>
#include <QScrollArea>
#include <vector>

namespace ui {

/**
 * @brief Panel de control con opciones de configuración
 * 
 * Características según INDICACIONES.md:
 * - Modo: [TSP, Ruta Corta] - default TSP
 * - Perfil: [Sin Restricciones, Automóvil, Peatón] - default Sin Restricciones
 * - Algoritmo de Ruta Corta: [Dijkstra, A*, ALT] - default Dijkstra
 * - Algoritmo TSP: [IG, ILS_B, IGSA, ...] - default IG (solo en modo TSP)
 * - Botones toggle para selección manual: Nodo inicio, Nodo/s destino
 * - Volver a inicio: checkbox (solo en modo TSP)
 * 
 * Al cambiar de modo, se resetean opciones excepto Perfil y Algoritmo de Ruta Corta
 */
class ControlPanel : public QWidget {
    Q_OBJECT

public:
    explicit ControlPanel(QWidget* parent = nullptr);
    ~ControlPanel();

    /**
     * @brief Habilita/deshabilita controles según estado de carga del grafo
     */
    void setGraphLoaded(bool loaded);

    /**
     * @brief Habilita/deshabilita el botón limpiar según si hay selección
     */
    void setHasSelection(bool hasSelection);

signals:
    /**
     * @brief Emitido cuando cambia el modo de operación
     * @param isTsp true para TSP, false para Pathfinding
     */
    void modeChanged(bool isTsp);

    /**
     * @brief Emitido cuando cambia el perfil de vehículo
     * @param profile "Sin Restricciones", "Automovil", o "Peaton"
     */
    void profileChanged(const QString& profile);

    /**
     * @brief Emitido cuando cambia el algoritmo de pathfinding
     * @param algorithm "dijkstra", "astar", o "alt"
     */
    void pathfindingAlgorithmChanged(const QString& algorithm);

    /**
     * @brief Emitido cuando cambia el algoritmo TSP
     * @param algorithm "ig", "ils_b", "igsa", etc.
     */
    void tspAlgorithmChanged(const QString& algorithm);

    /**
     * @brief Emitido cuando cambia el estado del checkbox "Volver a inicio"
     * @param returnToStart true si debe volver al inicio
     */
    void returnToStartChanged(bool returnToStart);

    /**
     * @brief Emitido cuando se activa/desactiva selección manual de inicio
     * @param active true si se activó, false si se desactivó
     */
    void manualStartSelectionChanged(bool active);

    /**
     * @brief Emitido cuando se activa/desactiva selección manual de destinos
     * @param active true si se activó, false si se desactivó
     */
    void manualDestSelectionChanged(bool active);

    /**
     * @brief Emitido cuando usuario solicita calcular ruta
     * @param nodeIds IDs de nodos seleccionados (2 para pathfinding, N para TSP)
     */
    void calculateRequested(const std::vector<int64_t>& nodeIds);

    /**
     * @brief Emitido cuando usuario solicita limpiar la selección
     */
    void clearSelectionRequested();

private slots:
    void onModeChanged(int index);
    void onProfileChanged(int index);
    void onPathfindingAlgorithmChanged(int index);
    void onTspAlgorithmChanged(int index);
    void onReturnToStartToggled(bool checked);
    void onManualStartButtonClicked();
    void onManualDestButtonClicked();
    void onCalculateButtonClicked();
    void onClearButtonClicked();

private:
    void setupUi();
    void updateVisibilityForMode();
    void resetSelectionsForModeChange();
    void deselectAllManualButtons();

    // === CONTROLES ===
    
    // Modo
    QComboBox* modeCombo_;
    
    // Perfil
    QComboBox* profileCombo_;
    
    // Algoritmos
    QComboBox* pathfindingAlgorithmCombo_;
    QComboBox* tspAlgorithmCombo_;
    QGroupBox* tspAlgorithmGroup_;  // Para ocultar/mostrar según modo
    
    // Selección manual
    QPushButton* manualStartButton_;
    QPushButton* manualDestButton_;
    QGroupBox* manualSelectionGroup_;
    
    // Volver a inicio (TSP)
    QCheckBox* returnToStartCheckbox_;
    QGroupBox* tspOptionsGroup_;  // Para ocultar/mostrar según modo
    
    // Acción
    QPushButton* calculateButton_;
    QPushButton* clearButton_; //---Limpiar nodos
    
    // === ESTADO ===
    bool graphLoaded_;
    bool isTspMode_;
};

} // namespace ui

#endif // CONTROLPANEL_H
