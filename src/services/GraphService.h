#pragma once

#include "../core/entities/Graph.h"
#include <QObject>
#include <QString>
#include <QFuture>
#include <atomic>
#include <memory>

namespace services {

/** 
 * Fallback's logic:
 * 1. Try to load .bin (fast, 1-3s)
 * 2. If it doesn't exist, load .osm (slow, 5-30s) and generate .bin
 * 3. If .osm doesn't exist, error
 */
class GraphService : public QObject {
    Q_OBJECT

private: 
    std::shared_ptr<Graph> graph;
    std::atomic<bool> cancelRequested = {false};
    QFuture<void> loadFuture_;

    // Loads graph (executing in separate thread)
    void loadGraphInternal(const QString& baseName);

    bool fileExists(const QString& path) const;

public:
    explicit GraphService(QObject* parent = nullptr);
    ~GraphService();

    /**
     * Load graph in async mode
     * @param baseName Base name without extension (e.g., "arequipa")
     * 
     * Searches in this order:
     * 1. data/graphs/{baseName}.bin
     * 2. data/maps/{baseName}.osm
     */
    void loadGraphAsync(const QString& baseName);

    // Cancel loading process
    void cancelLoad();

    // Return the graph loaded (return nullptr if there isn't any)
    std::shared_ptr<Graph> getGraph() const;

signals:
    // Emited when the graph is loaded
    void graphLoaded(const std::shared_ptr<Graph>& graph, qint64 loadTimeMs);

    /**
     * Emited to report progress during loading
     * @param message Status message
     * @param progress Progress [0.0 - 1.0]
     */
    void loadProgress(const QString& message, double progress);

    // Emited when an error occurs during loading
    void loadError(const QString& errorMessage);
};

}