#ifndef MAPWIDGET_H
#define MAPWIDGET_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QGraphicsPathItem>
#include <QPixmap>
#include <QTimer>
#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>

class Graph;
class Edge;

namespace ui {

/**
 * @brief Modo de selección de nodos
 */
enum class SelectionMode {
    AUTOMATIC,      // Selección automática por defecto
    MANUAL_START,   // Selección manual de nodo inicio
    MANUAL_DEST     // Selección manual de nodos destino
};

/**
 * @brief Modo de operación (TSP o Ruta Corta)
 */
enum class OperationMode {
    PATHFINDING,    
    TSP 
};

/**
 * @brief Calidad de renderizado de aristas
 */
enum class RenderQuality {
    LOW = 0,      // Sin antialiasing, render rápido, calidad baja
    MEDIUM = 1,   // Antialiasing moderado, balance velocidad/calidad
    HIGH = 2      // Máximo antialiasing, render lento, máxima nitidez
};

/**
 * @brief Widget de visualización del mapa con selección de nodos
 */
class MapWidget : public QGraphicsView
{
    Q_OBJECT

public:
    explicit MapWidget(QWidget* parent = nullptr);
    ~MapWidget();

    /**
     * @brief Establece el grafo a visualizar
     */
    void setGraph(std::shared_ptr<Graph> graph);

    /**
     * @brief Establece el perfil de vehículo actual
     */
    void setVehicleProfile(const QString& profile);

    /**
     * @brief Establece el modo de operación (Pathfinding o TSP)
     */
    void setOperationMode(OperationMode mode);

    /**
     * @brief Establece el modo de selección de nodos
     */
    void setSelectionMode(SelectionMode mode);

    /**
     * @brief Establece la calidad de renderizado
     */
    void setRenderQuality(RenderQuality quality);

    /**
     * @brief Muestra ruta en el mapa (color azul)
     */
    void displayPath(const std::vector<Edge*>& pathEdges);

    /**
     * @brief Muestra tour TSP en el mapa
     */
    void displayTspTour(const std::vector<std::vector<Edge*>>& segmentEdges, int currentSegmentIndex = -1);

    /**
     * @brief Limpia visualización de rutas
     */
    void clearPath();

    /**
     * @brief Obtiene los nodos seleccionados actualmente
     */
    std::vector<int64_t> getSelectedNodes() const { return selectedNodeIds_; }

    /**
     * @brief Limpia selección de nodos
     */
    void clearSelection();

signals:
    /**
     * @brief Emitido cuando usuario selecciona/deselecciona nodo
     */
    void nodeSelected(int64_t nodeId, bool isSelected);

    /**
     * @brief Emitido cuando selección de nodos cambia
     */
    void selectionChanged(const std::vector<int64_t>& selectedNodes);

protected:
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    void drawEdge(Edge* edge, const QColor& color, double width);
    void drawNode(int64_t nodeId, const QColor& color, double radius);
    void drawSelectedNodes();
    void updateNodeLabels();
    
    /**
     * @brief Encuentra nodo cercano a posición del mouse
     */
    int64_t findNodeAtPosition(const QPointF& scenePos, double threshold = 15.0);

    /**
     * @brief Convierte coordenadas geográficas (lat, lon) a píxeles
     */
    QPointF geoToPixel(double lat, double lon) const;

    /**
     * @brief Verifica si una arista está bloqueada según el perfil actual
     */
    bool isEdgeBlocked(Edge* edge) const;

    /**
     * @brief Maneja click en nodo (selección/deselección)
     */
    void handleNodeClick(int64_t nodeId);

    /**
     * @brief Limpia zoom y pan
     */
    void resetView();

    QGraphicsScene* scene_;
    std::shared_ptr<Graph> graph_;
    
    // Transformación de coordenadas
    double minLat_, maxLat_, minLon_, maxLon_;
    double scaleX_, scaleY_;
    
    // Estado de zoom
    double currentZoom_;
    static constexpr double MIN_ZOOM = 0.5;
    static constexpr double MAX_ZOOM = 15.0;
    static constexpr double INITIAL_ZOOM = 3.0;
    
    // Estado de pan
    bool isPanning_;
    QPoint lastPanPoint_;
    
    // Perfil de vehículo
    QString vehicleProfile_;
    
    // Modo de operación y selección
    OperationMode operationMode_;
    SelectionMode selectionMode_;
    
    // Nodos seleccionados (en orden de selección)
    std::vector<int64_t> selectedNodeIds_;
    
    // Items gráficos de nodos seleccionados (para borrado rápido)
    std::unordered_map<int64_t, QGraphicsEllipseItem*> selectedNodeItems_;
    std::unordered_map<int64_t, QGraphicsTextItem*> nodeLabelItems_;
    
    // Rutas visualizadas actualmente
    std::vector<Edge*> currentPathEdges_;
    std::vector<std::vector<Edge*>> currentTspSegments_;
    int currentHighlightedSegment_;
    
    // Items de aristas renderizadas
    std::vector<QGraphicsLineItem*> edgeItems_;
    
    // Calidad de renderizado
    RenderQuality renderQuality_ = RenderQuality::MEDIUM;
    
    // Cache de colores para performance
    std::unordered_map<std::string, QColor> colorCache_;
    QString lastVehicleProfile_;
    
    // Spatial grid para renderizado eficiente
    static constexpr int GRID_SIZE = 50;
    std::vector<std::vector<std::vector<Edge*>>> spatialGrid_;
    bool gridBuilt_ = false;
    
    // Aristas bloqueadas por perfil (precalculadas)
    std::unordered_set<Edge*> blockedForAutomovil_;
    std::unordered_set<Edge*> blockedForPeaton_;
    bool edgesClassified_ = false;
    
    void classifyBlockedEdges();
    void buildSpatialGrid();
    void renderVisibleEdges();
    QColor getHighwayColor(Edge* edge);
    int getGridX(double x) const;
    int getGridY(double y) const;
};

} // namespace ui

#endif // MAPWIDGET_H