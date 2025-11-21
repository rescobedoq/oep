#ifndef MAPWIDGET_H
#define MAPWIDGET_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <memory>
#include "core/entities/Graph.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MapWidget;
}
QT_END_NAMESPACE

class MapWidget : public QGraphicsView
{
    Q_OBJECT

public:
    explicit MapWidget(QWidget* parent = nullptr);
    ~MapWidget();

    /**
     * @brief Establece el grafo a visualizar
     */
    void setGraph(std::shared_ptr<core::entities::Graph> graph);

    /**
     * @brief Muestra ruta en el mapa
     * @param edgeIds IDs de aristas de la ruta
     */
    void displayPath(const std::vector<int64_t>& edgeIds);

    /**
     * @brief Muestra tour TSP en el mapa
     * @param tour Índices de nodos en orden de visita
     */
    void displayTspTour(const std::vector<int>& tour);

    /**
     * @brief Limpia visualización de rutas
     */
    void clearPath();

protected:
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    void drawGraph();
    void drawEdge(std::shared_ptr<core::entities::Edge> edge, const QColor& color, double width);
    void drawNode(std::shared_ptr<core::entities::Node> node, const QColor& color, double radius);

    QGraphicsScene* scene_;
    std::shared_ptr<core::entities::Graph> graph_;

    // Transformación de coordenadas geográficas a píxeles
    double minLat_, maxLat_, minLon_, maxLon_;
    double scaleX_, scaleY_;

    // Estado de pan
    bool isPanning_;
    QPoint lastPanPoint_;

    // Rutas visualizadas
    std::vector<int64_t> currentPathEdgeIds_;
};


#endif // MAPWIDGET_H
