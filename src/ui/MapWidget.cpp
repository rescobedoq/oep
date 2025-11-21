#include "MapWidget.h"
#include <QWheelEvent>
#include <QGraphicsLineItem>
#include <QGraphicsEllipseItem>
#include <QPen>
#include <QBrush>
#include <QDebug>
#include <cmath>

MapWidget::MapWidget(QWidget* parent)
    : QGraphicsView(parent),
      isPanning_(false) {
    
    scene_ = new QGraphicsScene(this);
    setScene(scene_);
    
    // Configuración de visualización
    setRenderHint(QPainter::Antialiasing);
    setDragMode(QGraphicsView::NoDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorUnderMouse);
    
    // Fondo blanco
    setBackgroundBrush(QBrush(Qt::white));
}

MapWidget::~MapWidget() = default;

void MapWidget::setGraph(std::shared_ptr<core::entities::Graph> graph) {
    graph_ = graph;
    
    if (!graph_) return;
    
    // Obtener bounding box
    auto bbox = graph_->getBoundingBox();
    minLat_ = bbox.minLat;
    maxLat_ = bbox.maxLat;
    minLon_ = bbox.minLon;
    maxLon_ = bbox.maxLon;
    
    // Calcular escala (coordenadas geográficas → píxeles)
    scaleX_ = 10000.0 / (maxLon_ - minLon_);
    scaleY_ = 10000.0 / (maxLat_ - minLat_);
    
    qDebug() << "Configurando mapa:";
    qDebug() << "   BBox:" << minLat_ << "," << minLon_ << "→" << maxLat_ << "," << maxLon_;
    qDebug() << "   Escala:" << scaleX_ << "×" << scaleY_;
    
    drawGraph();
}

void MapWidget::drawGraph() {
    scene_->clear();
    
    if (!graph_) return;
    
    auto edges = graph_->getEdges();
    auto nodes = graph_->getNodes();
    
    qDebug() << "Dibujando grafo:" << edges.size() << "aristas";
    
    // Dibujar aristas (gris claro)
    for (const auto& [id, edge] : edges) {
        drawEdge(edge, QColor(200, 200, 200), 1.0);
    }
    
    // Ajustar vista al contenido
    scene_->setSceneRect(scene_->itemsBoundingRect());
    fitInView(scene_->sceneRect(), Qt::KeepAspectRatio);
}

void MapWidget::drawEdge(std::shared_ptr<core::entities::Edge> edge, const QColor& color, double width) {
    if (!graph_) return;
    
    auto fromNode = graph_->getNode(edge->getFromId());
    auto toNode = graph_->getNode(edge->getToId());
    
    if (!fromNode || !toNode) return;
    
    // Convertir coordenadas geográficas a píxeles
    double x1 = (fromNode->getLon() - minLon_) * scaleX_;
    double y1 = (maxLat_ - fromNode->getLat()) * scaleY_; // Invertir Y
    double x2 = (toNode->getLon() - minLon_) * scaleX_;
    double y2 = (maxLat_ - toNode->getLat()) * scaleY_;
    
    QPen pen(color);
    pen.setWidthF(width);
    
    scene_->addLine(x1, y1, x2, y2, pen);
}

void MapWidget::drawNode(std::shared_ptr<core::entities::Node> node, const QColor& color, double radius) {
    double x = (node->getLon() - minLon_) * scaleX_;
    double y = (maxLat_ - node->getLat()) * scaleY_;
    
    QPen pen(color.darker());
    pen.setWidthF(1.0);
    QBrush brush(color);
    
    scene_->addEllipse(x - radius, y - radius, radius * 2, radius * 2, pen, brush);
}

void MapWidget::displayPath(const std::vector<int64_t>& edgeIds) {
    clearPath();
    
    if (!graph_) return;
    
    currentPathEdgeIds_ = edgeIds;
    
    qDebug() << "Mostrando ruta:" << edgeIds.size() << "aristas";
    
    // Redibujar grafo completo (fondo)
    drawGraph();
    
    // Dibujar ruta resaltada (azul, más gruesa)
    for (int64_t edgeId : edgeIds) {
        auto edge = graph_->getEdge(edgeId);
        if (edge) {
            drawEdge(edge, QColor(0, 100, 255), 3.0);
        }
    }
    
    // Dibujar nodos inicio (verde) y fin (rojo)
    if (!edgeIds.empty()) {
        auto firstEdge = graph_->getEdge(edgeIds.front());
        auto lastEdge = graph_->getEdge(edgeIds.back());
        
        if (firstEdge) {
            auto startNode = graph_->getNode(firstEdge->getFromId());
            if (startNode) drawNode(startNode, QColor(0, 200, 0), 8.0);
        }
        
        if (lastEdge) {
            auto endNode = graph_->getNode(lastEdge->getToId());
            if (endNode) drawNode(endNode, QColor(200, 0, 0), 8.0);
        }
    }
}

void MapWidget::displayTspTour(const std::vector<int>& tour) {
    clearPath();
    
    if (!graph_ || tour.empty()) return;
    
    qDebug() << "🔄 Mostrando tour TSP:" << tour.size() << "nodos";
    
    // Redibujar grafo completo
    drawGraph();
    
    // Dibujar tour como secuencia de aristas
    // (Aquí simplificamos dibujando los nodos en orden)
    
    for (size_t i = 0; i < tour.size(); ++i) {
        // tour[i] es un índice, necesitamos convertir a nodeId
        // Por ahora dibujamos solo los nodos
        // TODO: Conectar con aristas reales del grafo
    }
    
    // Dibujar nodo de inicio destacado
    // (Implementación simplificada)
}

void MapWidget::clearPath() {
    currentPathEdgeIds_.clear();
    
    if (graph_) {
        drawGraph();
    }
}

void MapWidget::wheelEvent(QWheelEvent* event) {
    // Zoom con rueda del mouse
    double scaleFactor = 1.15;
    
    if (event->angleDelta().y() > 0) {
        scale(scaleFactor, scaleFactor);
    } else {
        scale(1.0 / scaleFactor, 1.0 / scaleFactor);
    }
    
    event->accept();
}

void MapWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
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
        event->accept();
    } else {
        QGraphicsView::mouseReleaseEvent(event);
    }
}