#include "MapWidget.h"
#include "src/core/entities/Graph.h"
#include "src/core/entities/Edge.h"
#include "src/core/entities/Node.h"
#include <QWheelEvent>
#include <QMouseEvent>
#include <QPen>
#include <QBrush>
#include <QScrollBar>
#include <QDebug>
#include <QPainter>
#include <QPixmap>
#include <cmath>
#include <algorithm>

namespace ui {

MapWidget::MapWidget(QWidget* parent)
    : QGraphicsView(parent),
      currentZoom_(INITIAL_ZOOM),
      isPanning_(false),
      vehicleProfile_("Sin Restricciones"),
      operationMode_(OperationMode::TSP),
      selectionMode_(SelectionMode::AUTOMATIC),
      currentHighlightedSegment_(-1) {
    
    scene_ = new QGraphicsScene(this);
    setScene(scene_);
    
    // Configuración de visualización
    setDragMode(QGraphicsView::NoDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorUnderMouse);
    
    // calidad de renderizado inicial (MEDIUM por defecto)
    setRenderQuality(RenderQuality::MEDIUM);
    
    // Modo de actualización más rápido
    setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
    
    // Cachear fondo
    setCacheMode(QGraphicsView::CacheBackground);
    
    // Fondo blanco
    setBackgroundBrush(QBrush(Qt::white));
}

MapWidget::~MapWidget() = default;

void MapWidget::setGraph(std::shared_ptr<Graph> graph) {
    graph_ = graph;
    
    if (!graph_) return;
    
    // Obtener bounding box
    auto [minLat, maxLat, minLon, maxLon] = graph_->getBounds();
    minLat_ = minLat;
    maxLat_ = maxLat;
    minLon_ = minLon;
    maxLon_ = maxLon;
    
    // Calcular escala (coordenadas geográficas → píxeles)
    scaleX_ = 10000.0 / (maxLon_ - minLon_);
    scaleY_ = 10000.0 / (maxLat_ - minLat_);
    
    qDebug() << "MapWidget: Grafo configurado";
    qDebug() << "   BBox:" << minLat_ << "," << minLon_ << "→" << maxLat_ << "," << maxLon_;
    qDebug() << "   Escala:" << scaleX_ << "×" << scaleY_;
    
    // Clasificar aristas bloqueadas UNA VEZ
    classifyBlockedEdges();
    
    // Construir grid para búsqueda rápida
    buildSpatialGrid();
    
    // Establecer vista
    resetView();
    
    // Configurar calidad inicial (se aplicará en setRenderQuality)
    // Renderizar viewport inicial
    renderVisibleEdges();
}

void MapWidget::setVehicleProfile(const QString& profile) {
    vehicleProfile_ = profile;
    
    if (graph_) {
        if (lastVehicleProfile_ != profile) {
            colorCache_.clear();
            lastVehicleProfile_ = profile;
        }
        renderVisibleEdges();
    }
}

void MapWidget::setOperationMode(OperationMode mode) {
    if (operationMode_ != mode) {
        operationMode_ = mode;
        clearSelection();
    }
}

void MapWidget::setSelectionMode(SelectionMode mode) {
    selectionMode_ = mode;
}

QPointF MapWidget::geoToPixel(double lat, double lon) const {
    double x = (lon - minLon_) * scaleX_;
    double y = (maxLat_ - lat) * scaleY_; // Invertir Y
    return QPointF(x, y);
}

bool MapWidget::isEdgeBlocked(Edge* edge) const {
    if (!edge || vehicleProfile_ == "Sin Restricciones") {
        return false;
    }
    
    // Usar clasificación precalculada
    if (vehicleProfile_ == "Automovil") {
        return blockedForAutomovil_.count(edge) > 0;
    } else if (vehicleProfile_ == "Peaton") {
        return blockedForPeaton_.count(edge) > 0;
    }
    
    return false;
}

// Clasificar aristas bloqueadas UNA VEZ al cargar
void MapWidget::classifyBlockedEdges() {
    if (!graph_ || edgesClassified_) return;
    
    qDebug() << "Clasificando aristas bloqueadas...";
    
    blockedForAutomovil_.clear();
    blockedForPeaton_.clear();
    
    auto edges = graph_->getEdges();
    
    for (Edge* edge : edges) {
        if (!edge) continue;
        
        auto highway = edge->getTags().find("highway");
        if (highway == edge->getTags().end()) continue;
        
        const std::string& highwayType = highway->second;
        
        // Clasificar para Automóvil
        if (highwayType == "footway" || 
            highwayType == "steps" || 
            highwayType == "path" ||
            highwayType == "pedestrian") {
            blockedForAutomovil_.insert(edge);
        }
        
        // Clasificar para Peatón
        if (highwayType == "motorway" || 
            highwayType == "motorway_link" || 
            highwayType == "trunk" ||
            highwayType == "trunk_link") {
            blockedForPeaton_.insert(edge);
        }
    }
    
    edgesClassified_ = true;
    
    qDebug() << "Clasificación completa:";
    qDebug() << "   Bloqueadas para Automóvil:" << blockedForAutomovil_.size();
    qDebug() << "   Bloqueadas para Peatón:" << blockedForPeaton_.size();
}

// Construir spatial grid para búsqueda O(1)
void MapWidget::buildSpatialGrid() {
    if (!graph_ || gridBuilt_) return;
    
    qDebug() << "Construyendo spatial grid" << GRID_SIZE << "x" << GRID_SIZE << "...";
    
    // Inicializar grid
    spatialGrid_.resize(GRID_SIZE);
    for (int i = 0; i < GRID_SIZE; ++i) {
        spatialGrid_[i].resize(GRID_SIZE);
    }
    
    // Insertar cada arista en las celdas que intersecta
    const auto& edges = graph_->getEdges();
    for (auto* edge : edges) {
        if (!edge) continue;
        
        auto fromNode = edge->getSource();
        auto toNode = edge->getTarget();
        if (!fromNode || !toNode) continue;
        
        QPointF p1 = geoToPixel(fromNode->getCoordinate().getLatitude(),
                                fromNode->getCoordinate().getLongitude());
        QPointF p2 = geoToPixel(toNode->getCoordinate().getLatitude(),
                                toNode->getCoordinate().getLongitude());
        
        // Obtener rango de celdas que cubre esta arista
        int minGridX = getGridX(qMin(p1.x(), p2.x()));
        int maxGridX = getGridX(qMax(p1.x(), p2.x()));
        int minGridY = getGridY(qMin(p1.y(), p2.y()));
        int maxGridY = getGridY(qMax(p1.y(), p2.y()));
        
        // Insertar en todas las celdas intersectadas
        for (int gx = minGridX; gx <= maxGridX; ++gx) {
            for (int gy = minGridY; gy <= maxGridY; ++gy) {
                if (gx >= 0 && gx < GRID_SIZE && gy >= 0 && gy < GRID_SIZE) {
                    spatialGrid_[gx][gy].push_back(edge);
                }
            }
        }
    }
    
    // Estadísticas
    int totalCells = GRID_SIZE * GRID_SIZE;
    int nonEmptyCells = 0;
    int maxEdgesInCell = 0;
    for (int i = 0; i < GRID_SIZE; ++i) {
        for (int j = 0; j < GRID_SIZE; ++j) {
            int count = spatialGrid_[i][j].size();
            if (count > 0) {
                nonEmptyCells++;
                maxEdgesInCell = qMax(maxEdgesInCell, count);
            }
        }
    }
    
    qDebug() << "Grid construido:" << nonEmptyCells << "/" << totalCells << "celdas ocupadas, máx" << maxEdgesInCell << "aristas/celda";
    
    gridBuilt_ = true;
}

// Configurar calidad de renderizado
void MapWidget::setRenderQuality(RenderQuality quality) {
    renderQuality_ = quality;
    
    // Aplicar render hints según calidad
    switch (quality) {
        case RenderQuality::LOW:
            // Baja calidad: SIN antialiasing, render rápido
            setRenderHint(QPainter::Antialiasing, false);
            setRenderHint(QPainter::TextAntialiasing, false);
            setRenderHint(QPainter::SmoothPixmapTransform, false);
            qDebug() << "Calidad de renderizado: BAJA (sin antialiasing)";
            break;
            
        case RenderQuality::MEDIUM:
            // Calidad media: Antialiasing moderado
            setRenderHint(QPainter::Antialiasing, true);
            setRenderHint(QPainter::TextAntialiasing, false);
            setRenderHint(QPainter::SmoothPixmapTransform, false);
            qDebug() << "Calidad de renderizado: MEDIA (antialiasing moderado)";
            break;
            
        case RenderQuality::HIGH:
            // Alta calidad: Máximo antialiasing
            setRenderHint(QPainter::Antialiasing, true);
            setRenderHint(QPainter::TextAntialiasing, true);
            setRenderHint(QPainter::SmoothPixmapTransform, true);
            qDebug() << "Calidad de renderizado: ALTA (máximo antialiasing)";
            break;
    }
    
    // Re-renderizar con nueva calidad
    if (graph_ && gridBuilt_) {
        renderVisibleEdges();
    }
}

int MapWidget::getGridX(double x) const {
    return qBound(0, static_cast<int>((x / 10000.0) * GRID_SIZE), GRID_SIZE - 1);
}

int MapWidget::getGridY(double y) const {
    return qBound(0, static_cast<int>((y / 10000.0) * GRID_SIZE), GRID_SIZE - 1);
}

// Cache de colores O(1)
QColor MapWidget::getHighwayColor(Edge* edge) {
    if (!edge) return QColor(200, 200, 200, 180);
    
    std::string highway = edge->getTags().count("highway") > 0 ? edge->getTags().at("highway") : "";
    std::string cacheKey = highway + "_" + vehicleProfile_.toStdString();
    
    // Verificar cache primero
    auto it = colorCache_.find(cacheKey);
    if (it != colorCache_.end()) {
        return it->second;
    }
    
    // Calcular color si no está en cache
    bool blocked = false;
    if (vehicleProfile_ == "Automovil") {
        blocked = blockedForAutomovil_.count(edge) > 0;
    } else if (vehicleProfile_ == "Peaton") {
        blocked = blockedForPeaton_.count(edge) > 0;
    }
    
    QColor color = blocked ? QColor(255, 80, 80, 180) : QColor(200, 200, 200, 180);
    colorCache_[cacheKey] = color;
    return color;
}

void MapWidget::drawEdge(Edge* edge, const QColor& color, double width) {
    if (!graph_ || !edge) return;
    
    auto fromNode = edge->getSource();
    auto toNode = edge->getTarget();
    
    if (!fromNode || !toNode) return;
    
    QPointF p1 = geoToPixel(fromNode->getCoordinate().getLatitude(), fromNode->getCoordinate().getLongitude());
    QPointF p2 = geoToPixel(toNode->getCoordinate().getLatitude(), toNode->getCoordinate().getLongitude());
    
    QPen pen(color);
    pen.setWidthF(width);
    pen.setCapStyle(Qt::RoundCap);
    
    scene_->addLine(p1.x(), p1.y(), p2.x(), p2.y(), pen);
}

void MapWidget::drawNode(int64_t nodeId, const QColor& color, double radius) {
    if (!graph_) return;
    
    auto node = graph_->getNode(nodeId);
    if (!node) return;
    
    QPointF pos = geoToPixel(node->getCoordinate().getLatitude(), node->getCoordinate().getLongitude());
    
    QPen pen(color.darker());
    pen.setWidthF(2.0);
    QBrush brush(color);
    
    auto* ellipse = scene_->addEllipse(
        pos.x() - radius, pos.y() - radius, 
        radius * 2, radius * 2, 
        pen, brush
    );
    ellipse->setZValue(10); // Nodos encima de aristas
}

void MapWidget::drawSelectedNodes() {
    // Limpiar items anteriores
    for (auto& [id, item] : selectedNodeItems_) {
        if (item && item->scene() == scene_) {
            scene_->removeItem(item);
        }
        delete item;
    }
    selectedNodeItems_.clear();
    
    for (auto& [id, item] : nodeLabelItems_) {
        if (item && item->scene() == scene_) {
            scene_->removeItem(item);
        }
        delete item;
    }
    nodeLabelItems_.clear();
    
    // Dibujar nodos seleccionados
    for (size_t i = 0; i < selectedNodeIds_.size(); ++i) {
        int64_t nodeId = selectedNodeIds_[i];
        auto node = graph_->getNode(nodeId);
        if (!node) continue;
        
        QPointF pos = geoToPixel(node->getCoordinate().getLatitude(), node->getCoordinate().getLongitude());
        
        // Primer nodo (inicio) en verde, resto en colores distintos
        QColor color = (i == 0) ? QColor(0, 200, 0) : QColor(255, 150, 0);
        double radius = 10.0;
        
        QPen pen(color.darker());
        pen.setWidthF(3.0);
        QBrush brush(color);
        
        auto* ellipse = scene_->addEllipse(
            pos.x() - radius, pos.y() - radius,
            radius * 2, radius * 2,
            pen, brush
        );
        ellipse->setZValue(20);
        selectedNodeItems_[nodeId] = ellipse;
        
        // Agregar letra solo para nodos destino (no para el inicio)
        if (operationMode_ == OperationMode::TSP && i > 0) {
            char letter = 'A' + static_cast<char>(i - 1);
            
            auto* textItem = scene_->addText(QString(letter));
            textItem->setDefaultTextColor(Qt::white);
            textItem->setFont(QFont("Arial", 12, QFont::Bold));
            
            // Centrar texto en el nodo
            QRectF textRect = textItem->boundingRect();
            textItem->setPos(
                pos.x() - textRect.width() / 2,
                pos.y() - textRect.height() / 2
            );
            textItem->setZValue(30);
            nodeLabelItems_[nodeId] = textItem;
        }
    }
}

void MapWidget::updateNodeLabels() {
    drawSelectedNodes();
}

int64_t MapWidget::findNodeAtPosition(const QPointF& scenePos, double threshold) {
    if (!graph_) return -1;
    
    double minDistSq = threshold * threshold;
    int64_t closestNodeId = -1;
    
    // Buscar nodo más cercano
    for (Node* node : graph_->getNodes()) {
        QPointF nodePos = geoToPixel(node->getCoordinate().getLatitude(), node->getCoordinate().getLongitude());
        double dx = scenePos.x() - nodePos.x();
        double dy = scenePos.y() - nodePos.y();
        double distSq = dx * dx + dy * dy;
        
        if (distSq < minDistSq) {
            minDistSq = distSq;
            closestNodeId = node->getId();
        }
    }
    
    return closestNodeId;
}

void MapWidget::handleNodeClick(int64_t nodeId) {
    if (nodeId == -1) return;
    
    // Verificar si el nodo ya está seleccionado
    auto it = std::find(selectedNodeIds_.begin(), selectedNodeIds_.end(), nodeId);
    bool isSelected = (it != selectedNodeIds_.end());
    
    if (isSelected) {
        selectedNodeIds_.erase(it);
    } else {
        if (operationMode_ == OperationMode::PATHFINDING) {
            if (selectedNodeIds_.size() >= 2) {
                selectedNodeIds_.clear();
            }
            selectedNodeIds_.push_back(nodeId);
        } else {
            selectedNodeIds_.push_back(nodeId);
        }
    }
    
    // Actualizar visualización
    drawSelectedNodes();
    
    // Emitir señales
    emit nodeSelected(nodeId, !isSelected);
    emit selectionChanged(selectedNodeIds_);
}

void MapWidget::clearSelection() {
    selectedNodeIds_.clear();
    drawSelectedNodes();
    emit selectionChanged(selectedNodeIds_);
}

void MapWidget::displayPath(const std::vector<Edge*>& pathEdges) {
    currentPathEdges_ = pathEdges;
    currentTspSegments_.clear();
    currentHighlightedSegment_ = -1;
    
    if (!graph_) return;
    
    for (auto* edge : pathEdges) {
        if (edge) {
            drawEdge(edge, QColor(0, 100, 255), 4.0);
        }
    }
    
    // Redibujar nodos seleccionados encima
    drawSelectedNodes();
}

void MapWidget::displayTspTour(
    const std::vector<std::vector<Edge*>>& segmentEdges,
    int currentSegmentIndex
) {
    currentTspSegments_ = segmentEdges;
    currentHighlightedSegment_ = currentSegmentIndex;
    currentPathEdges_.clear();
    
    if (!graph_) return;
    
    // Dibujar todos los segmentos
    for (size_t i = 0; i < segmentEdges.size(); ++i) {
        const auto& segment = segmentEdges[i];
        
        // Color: verde para segmento actual, azul para resto
        QColor color = (static_cast<int>(i) == currentSegmentIndex) ? 
                       QColor(0, 200, 0) : QColor(0, 100, 255);
        double width = (static_cast<int>(i) == currentSegmentIndex) ? 5.0 : 3.0;
        
        for (auto* edge : segment) {
            if (edge) {
                drawEdge(edge, color, width);
            }
        }
    }
    
    // Redibujar nodos seleccionados encima
    drawSelectedNodes();
}

void MapWidget::clearPath() {
    currentPathEdges_.clear();
    currentTspSegments_.clear();
    currentHighlightedSegment_ = -1;
    
    if (graph_) {
        renderVisibleEdges();
    }
}

void MapWidget::resetView() {
    if (!graph_) return;
    
    resetTransform();
    currentZoom_ = INITIAL_ZOOM;  // 🚀 Zoom inicial GRANDE (3.0)
    scale(currentZoom_, currentZoom_);
    
    scene_->setSceneRect(0, 0, 10000, 10000);
    centerOn(scene_->sceneRect().center());
}

void MapWidget::wheelEvent(QWheelEvent* event) {
    // Zoom con rueda del mouse
    double scaleFactor = 1.15;
    
    double newZoom = currentZoom_;
    
    if (event->angleDelta().y() > 0) {
        newZoom *= scaleFactor;
    } else {
        newZoom /= scaleFactor;
    }
    
    // Aplicar límites de zoom
    if (newZoom < MIN_ZOOM || newZoom > MAX_ZOOM) {
        event->accept();
        return;
    }
    
    // Aplicar zoom
    double actualScale = newZoom / currentZoom_;
    scale(actualScale, actualScale);
    currentZoom_ = newZoom;
    renderVisibleEdges();
    
    event->accept();
}

void MapWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        // Convertir posición a coordenadas de escena
        QPointF scenePos = mapToScene(event->pos());
        
        // Intentar encontrar nodo cercano
        int64_t nodeId = findNodeAtPosition(scenePos, 15.0);
        
        if (nodeId != -1 && selectionMode_ == SelectionMode::AUTOMATIC) {
            // Manejar selección de nodo
            handleNodeClick(nodeId);
            event->accept();
            return;
        } else if (nodeId != -1 && 
                   (selectionMode_ == SelectionMode::MANUAL_START || 
                    selectionMode_ == SelectionMode::MANUAL_DEST)) {
            // Selección manual
            handleNodeClick(nodeId);
            event->accept();
            return;
        }
        
        // Si no hay nodo, iniciar pan
        isPanning_ = true;
        lastPanPoint_ = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
    } else {
        QGraphicsView::mousePressEvent(event);
    }
}

void MapWidget::mouseMoveEvent(QMouseEvent* event) {
    if (isPanning_) {
        QPoint delta = event->pos() - lastPanPoint_;
        lastPanPoint_ = event->pos();
        
        // Pan horizontal y vertical
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
        
        event->accept();
    } else {
        QGraphicsView::mouseMoveEvent(event);
    }
}

void MapWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && isPanning_) {
        isPanning_ = false;
        setCursor(Qt::ArrowCursor);
        renderVisibleEdges();
        
        event->accept();
    } else {
        QGraphicsView::mouseReleaseEvent(event);
    }
}

void MapWidget::renderVisibleEdges() {
    if (!graph_ || !gridBuilt_) return;
    
    // Limpiar aristas anteriores
    for (auto* item : edgeItems_) {
        if (item && item->scene() == scene_) {
            scene_->removeItem(item);
        }
        delete item;
    }
    edgeItems_.clear();
    
    QRectF visibleRect = mapToScene(viewport()->rect()).boundingRect();
    
    // Buscar solo en celdas visibles del grid
    int minGridX = getGridX(visibleRect.left());
    int maxGridX = getGridX(visibleRect.right());
    int minGridY = getGridY(visibleRect.top());
    int maxGridY = getGridY(visibleRect.bottom());
    
    std::unordered_set<Edge*> candidates;
    for (int gx = minGridX; gx <= maxGridX; ++gx) {
        for (int gy = minGridY; gy <= maxGridY; ++gy) {
            if (gx >= 0 && gx < GRID_SIZE && gy >= 0 && gy < GRID_SIZE) {
                for (auto* edge : spatialGrid_[gx][gy]) {
                    candidates.insert(edge);
                }
            }
        }
    }
    
    // Renderizar aristas visibles
    int renderedCount = 0;
    for (auto* edge : candidates) {
        if (!edge) continue;
        
        auto fromNode = edge->getSource();
        auto toNode = edge->getTarget();
        if (!fromNode || !toNode) continue;
        
        QPointF p1 = geoToPixel(fromNode->getCoordinate().getLatitude(),fromNode->getCoordinate().getLongitude());
        QPointF p2 = geoToPixel(toNode->getCoordinate().getLatitude(),toNode->getCoordinate().getLongitude());
        
        QColor color = getHighwayColor(edge);
        
        bool blocked = (vehicleProfile_ == "Automovil" && blockedForAutomovil_.count(edge) > 0) ||
                      (vehicleProfile_ == "Peaton" && blockedForPeaton_.count(edge) > 0);
        double width = blocked ? 1.3 : 0.7;
        
        QPen pen(color);
        pen.setWidthF(width);
        pen.setCapStyle(Qt::RoundCap);
        
        auto* lineItem = scene_->addLine(p1.x(), p1.y(), p2.x(), p2.y(), pen);
        lineItem->setZValue(0);
        edgeItems_.push_back(lineItem);
        renderedCount++;
    }
    
    drawSelectedNodes();
    
    if (scene_->sceneRect().isEmpty()) {
        scene_->setSceneRect(0, 0, 10000, 10000);
    }
}

} // namespace ui
