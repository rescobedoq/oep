#include "ResultsPanel.h"
#include <QHeaderView>
#include <QTreeWidgetItem>
#include <cmath>

namespace ui {

ResultsPanel::ResultsPanel(QWidget* parent)
    : QWidget(parent) {
    setupUi();
}

ResultsPanel::~ResultsPanel() = default;

void ResultsPanel::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    
    // Título
    titleLabel_ = new QLabel("Resultados", this);
    QFont titleFont = titleLabel_->font();
    titleFont.setPointSize(12);
    titleFont.setBold(true);
    titleLabel_->setFont(titleFont);
    mainLayout->addWidget(titleLabel_);
    
    // Resumen (distancia total, tiempo total)
    summaryLabel_ = new QLabel("No hay resultados", this);
    summaryLabel_->setWordWrap(true);
    summaryLabel_->setStyleSheet("QLabel { padding: 10px; background-color: #f0f0f0; border-radius: 5px; }");
    mainLayout->addWidget(summaryLabel_);
    
    // Árbol de detalles por tramo
    detailsTree_ = new QTreeWidget(this);
    detailsTree_->setHeaderLabels({"Descripción", "Valor"});
    detailsTree_->setColumnWidth(0, 300);
    detailsTree_->setAlternatingRowColors(true);
    detailsTree_->setStyleSheet(R"(
        QTreeWidget {
            border: 1px solid #ccc;
            border-radius: 5px;
        }
        QTreeWidget::item {
            padding: 5px;
        }
        QTreeWidget::item:selected {
            background-color: #0078d4;
            color: white;
        }
    )");
    mainLayout->addWidget(detailsTree_);
    
    setLayout(mainLayout);
}

void ResultsPanel::clear() {
    summaryLabel_->setText("No hay resultados");
    detailsTree_->clear();
}

QString ResultsPanel::formatDistance(double meters) const {
    if (meters < 1000) {
        return QString("%1 m").arg(static_cast<int>(meters));
    } else {
        return QString("%1 km").arg(meters / 1000.0, 0, 'f', 2);
    }
}

QString ResultsPanel::formatTime(double milliseconds) const {
    double seconds = milliseconds / 1000.0;
    
    if (seconds < 60) {
        return QString("%1 seg").arg(static_cast<int>(seconds));
    } else {
        int minutes = static_cast<int>(seconds / 60);
        int secs = static_cast<int>(std::fmod(seconds, 60.0));
        return QString("%1 min %2 seg").arg(minutes).arg(secs);
    }
}

double ResultsPanel::calculateTime(double distanceMeters) const {
    // Tiempo = Distancia / Velocidad
    // Convertir velocidad de km/h a m/s
    double speedMs = (AVERAGE_SPEED_KMH * 1000.0) / 3600.0;
    double timeSeconds = distanceMeters / speedMs;
    return timeSeconds * 1000.0; // Retornar en milisegundos
}

void ResultsPanel::displayPathResult(const PathfindingService::PathResult& result) {
    detailsTree_->clear();
    
    // Resumen
    double timeMs = calculateTime(result.totalDistance);
    summaryLabel_->setText(QString(
        "<b> Distancia Total:</b> %1<br>"
        "<b>️ Tiempo Estimado:</b> %2<br>"
        "<b> Nodos Explorados:</b> %3<br>"
        "<b> Algoritmo:</b> %4"
    ).arg(formatDistance(result.totalDistance))
     .arg(formatTime(timeMs))
     .arg(result.nodesExplored)
     .arg(QString::fromStdString(result.algorithmName).toUpper()));
    
    // Crear ítem raíz para la ruta
    auto* routeItem = new QTreeWidgetItem(detailsTree_);
    routeItem->setText(0, "Ruta Completa");
    routeItem->setText(1, QString("%1 edges").arg(result.pathEdges.size()));
    routeItem->setExpanded(true);
    
    // Construir cadena de nodos: Inicio → A → B → Fin
    QString nodePath;
    if (!result.pathNodeIds.empty()) {
        nodePath = QString::number(result.pathNodeIds[0]);
        for (size_t i = 1; i < result.pathNodeIds.size(); ++i) {
            nodePath += " → " + QString::number(result.pathNodeIds[i]);
        }
    }
    
    auto* pathItem = new QTreeWidgetItem(routeItem);
    pathItem->setText(0, "Tramos");
    pathItem->setText(1, nodePath);
    
    // Detalles por cada edge (calle)
    auto* streetsItem = new QTreeWidgetItem(routeItem);
    streetsItem->setText(0, "Calles en Orden");
    streetsItem->setText(1, QString("%1 calles").arg(result.pathEdges.size()));
    streetsItem->setExpanded(true);
    
    for (size_t i = 0; i < result.pathEdges.size(); ++i) {
        Edge* edge = result.pathEdges[i];
        if (!edge) continue;
        
        double edgeDistance = edge->getDistance().getMeters();
        QString streetName = QString::fromStdString(edge->getStreetName());
        
        auto* streetItem = new QTreeWidgetItem(streetsItem);
        streetItem->setText(0, QString("%1. %2").arg(i + 1).arg(streetName));
        streetItem->setText(1, formatDistance(edgeDistance));
    }
}

void ResultsPanel::displayTspResult(const TspService::TspResult& result) {
    detailsTree_->clear();
    
    // Resumen
    double timeMs = calculateTime(result.totalDistance);
    summaryLabel_->setText(QString(
        "<b> Distancia Total del Tour:</b> %1<br>"
        "<b>️ Tiempo Estimado:</b> %2<br>"
        "<b> Nodos Visitados:</b> %3<br>"
        "<b> Algoritmo TSP:</b> %4"
    ).arg(formatDistance(result.totalDistance))
     .arg(formatTime(timeMs))
     .arg(result.nodeIds.size())
     .arg(QString::fromStdString(result.tspAlgorithmName).toUpper()));
    
    // Construir cadena de tour
    QString tourPath = "Inicio";
    for (size_t i = 1; i < result.tour.size(); ++i) {
        int nodeIdx = result.tour[i];
        char label = 'A' + (i - 1);
        tourPath += QString(" → %1").arg(label);
    }
    if (result.tour.size() > 1 && result.tour.front() == result.tour.back()) {
        tourPath += " → Inicio";
    }
    
    // Crear ítem raíz para el tour
    auto* tourItem = new QTreeWidgetItem(detailsTree_);
    tourItem->setText(0, "Tour TSP");
    tourItem->setText(1, QString("%1 tramos").arg(result.segmentEdges.size()));
    tourItem->setExpanded(true);
    
    auto* pathItem = new QTreeWidgetItem(tourItem);
    pathItem->setText(0, "Secuencia");
    pathItem->setText(1, tourPath);
    
    // Detalles por cada segmento (A → B, B → C, etc.)
    for (size_t segIdx = 0; segIdx < result.segmentEdges.size(); ++segIdx) {
        const auto& segmentEdges = result.segmentEdges[segIdx];
        const auto& segmentNodes = result.segmentNodes[segIdx];
        
        if (segmentEdges.empty()) continue;
        
        // Calcular distancia y tiempo del segmento
        double segmentDistance = 0.0;
        for (Edge* edge : segmentEdges) {
            if (edge) {
                segmentDistance += edge->getDistance().getMeters();
            }
        }
        double segmentTime = calculateTime(segmentDistance);
        
        // Etiquetas para el segmento
        QString fromLabel = (segIdx == 0) ? "Inicio" : QString(QChar('A' + static_cast<char>(segIdx - 1)));
        QString toLabel = QString(QChar('A' + static_cast<char>(segIdx)));
        
        // Si es el último segmento y vuelve al inicio
        if (segIdx == result.segmentEdges.size() - 1 && 
            !segmentNodes.empty() && segmentNodes.back() == result.nodeIds[0]) {
            toLabel = "Inicio";
        }
        
        auto* segmentItem = new QTreeWidgetItem(tourItem);
        segmentItem->setText(0, QString("Tramo %1 → %2").arg(fromLabel).arg(toLabel));
        segmentItem->setText(1, formatDistance(segmentDistance));
        segmentItem->setExpanded(false);
        
        // Distancia del tramo
        auto* distItem = new QTreeWidgetItem(segmentItem);
        distItem->setText(0, "Distancia total");
        distItem->setText(1, formatDistance(segmentDistance));
        
        // Tiempo del tramo
        auto* timeItem = new QTreeWidgetItem(segmentItem);
        timeItem->setText(0, "Tiempo total");
        timeItem->setText(1, formatTime(segmentTime));
        
        // Calles del tramo
        auto* streetsHeader = new QTreeWidgetItem(segmentItem);
        streetsHeader->setText(0, "️Calles en orden");
        streetsHeader->setText(1, QString("%1 calles").arg(segmentEdges.size()));
        streetsHeader->setExpanded(true);
        
        for (size_t i = 0; i < segmentEdges.size(); ++i) {
            Edge* edge = segmentEdges[i];
            if (!edge) continue;
            
            QString streetName = QString::fromStdString(edge->getStreetName());
            double edgeDistance = edge->getDistance().getMeters();
            
            auto* streetItem = new QTreeWidgetItem(streetsHeader);
            streetItem->setText(0, QString("      %1. %2").arg(i + 1).arg(streetName));
            streetItem->setText(1, formatDistance(edgeDistance));
        }
    }
}

} // namespace ui
