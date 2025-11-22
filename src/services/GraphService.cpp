#include "GraphService.h"
#include "../infraestructure/loaders/OSMGraphLoader.h"
#include "../infraestructure/loaders/BinaryGraphLoader.h"
#include "../infraestructure/loaders/BinaryGraphSerializer.h"
#include <QFile>
#include <QFileInfo>
#include <QElapsedTimer>
#include <QDebug>
#include <thread>
#include <stdexcept>

namespace services {

GraphService::GraphService(QObject* parent)
    : QObject(parent) {
}

GraphService::~GraphService() {
    cancelLoad();
}

void GraphService::loadGraphAsync(const QString& baseName) {
    cancelRequested = false;
    
    // Execute in a separate thread
    std::thread loadThread([this, baseName]() {
        loadGraphInternal(baseName);
    });
    loadThread.detach();
}

void GraphService::cancelLoad() {
    cancelRequested = true;
}

std::shared_ptr<Graph> GraphService::getGraph() const {
    return graph;
}

bool GraphService::fileExists(const QString& path) const {
    return QFileInfo::exists(path);
}

void GraphService::loadGraphInternal(const QString& baseName) {
    QElapsedTimer timer;
    timer.start();

    try {
        // FIRST: Try load from .bin (faster)
        QString binPath = QString("data/graphs/%1.bin").arg(baseName);
        
        if (fileExists(binPath)) {
            emit loadProgress("Loading graph from binary (Faster)...", 0.1);
            
            qDebug() << "Loading graph from .bin:" << binPath;

            auto loadedGraph = io::BinaryGraphLoader::load(binPath);

            if (cancelRequested) {
                emit loadError("Graph loading cancelled.");
                return;
            }

            graph = loadedGraph;    
            qint64 loadTime = timer.elapsed();
            
            // Debug messages
            qDebug() << "Graph loaded from .bin in" << loadTime << "ms";
            qDebug() << "Nodes:" << graph->getNodeCount() << ", Edges:" << graph->getEdgeCount();

            emit graphLoaded(graph, loadTime);
            return;
        }

        // SECOND: Try load from .osm (slower) and generate .bin
        QString osmPath = QString("data/maps/%1.osm").arg(baseName);

        if (!fileExists(osmPath)) {
            QString error = QString("No graph file found: %1 or %2").arg(binPath, osmPath);
            qDebug() << error;            
            emit loadError(error);
            return;
        }

        emit loadProgress("Loading graph from OSM (first time, slower)...", 0.2);

        qDebug() << "Loading graph from .osm:" << osmPath;
        qDebug() << "(This may take 5-30 seconds...)";

        // Load OSM with callbacks for progress
        auto progressCallback = [this](const QString& message, double progress) {
            if (!cancelRequested) {
                emit loadProgress(message, 0.2 + progress * 0.6); // 20-80%
            }
        };

        auto loadedGraph = io::OSMGraphLoader::load(osmPath, progressCallback);

        if (cancelRequested) {
            emit loadError("Graph loading cancelled.");
            return;
        }

        // THIRD: Save to .bin for future faster loads
        emit loadProgress("Saving graph to binary for future loads...", 0.85);
        qDebug() << "Generating .bin for next execution...";

        try {
            io::BinaryGraphSerializer::serialize(loadedGraph, binPath);
            qDebug() << "Binary generated:" << binPath;
        } catch (const std::exception& ex) {
            qWarning() << "Could not generate .bin:" << ex.what();
            qWarning() << "(Not critical, will read .osm again next time)";
        }

        graph = loadedGraph;
        qint64 loadTime = timer.elapsed();

        qDebug() << "Graph loaded from .osm in" << loadTime << "ms";
        qDebug() << "Nodes:" << graph->getNodeCount() << ", Edges:" << graph->getEdgeCount();
        
        emit loadProgress("Load complete", 1.0);
        emit graphLoaded(graph, loadTime);

    } catch (const std::exception& e) {
        QString errorMessage = QString("Error loading graph: %1").arg(e.what());
        qDebug() << errorMessage;
        emit loadError(errorMessage);
    }
}

}