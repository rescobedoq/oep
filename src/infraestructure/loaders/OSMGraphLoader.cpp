#include "OSMGraphLoader.h"
#include <QFile>
#include <QXmlStreamReader>
#include <QDebug>
#include <cmath>
#include <map>
#include <optional>
#include <unordered_map>
#include <limits>
#include <algorithm>

namespace services {
namespace io {

std::shared_ptr<Graph> OSMGraphLoader::load(
    const QString& filePath,
    ProgressCallback progressCallback
) {
    // Open OSM file
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw std::runtime_error("Could not open OSM file: " + filePath.toStdString());
    }

    QXmlStreamReader xml(&file);

    std::map<int64_t, std::shared_ptr<Node>> nodes;
    std::vector<std::shared_ptr<Edge>> edges;

    int64_t nextEdgeId = 1;
    int processedWays = 0;

    if (progressCallback) {
        progressCallback("Parsing OSM file...", 0.0);
    }

    qDebug() << "Starting OSM parsing for file:" << filePath;

    // FIRST: Parsing all nodes
    std::map<int64_t, std::pair<double, double>> nodePositions;

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement() && xml.name() == QString("node")) {
            QXmlStreamAttributes attributes = xml.attributes();
            int64_t id = attributes.value("id").toLongLong();
            double lat = attributes.value("lat").toDouble();
            double lon = attributes.value("lon").toDouble();

            nodePositions[id] = {lat, lon};
        }
    }

    qDebug() << "Total nodes parsed:" << nodePositions.size();

    if (progressCallback) {
        progressCallback("Parsed nodes. Now parsing ways...", 0.3);
    }

    // SECOND: Return to beginning to parse ways
    file.seek(0);
    xml.setDevice(&file);

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement() && xml.name() == QString("way")) {
            // Read way tags
            std::map<QString, QString> tags;
            std::vector<int64_t> wayNodes;

            while (!xml.atEnd()) {
                xml.readNext();

                if (xml.isEndElement() && xml.name() == QString("way")) {
                    break;
                }
                if (xml.isStartElement()) {
                    if (xml.name() == QString("way")) {
                        break;
                    }
                }

                if (xml.isStartElement()) {
                    if (xml.name() == QString("nd")) {
                        int64_t nodeRef = xml.attributes().value("ref").toLongLong();
                        wayNodes.push_back(nodeRef);
                    } else if (xml.name() == QString("tag")) {
                        QXmlStreamAttributes attributes = xml.attributes();
                        QString key = attributes.value("k").toString();
                        QString value = attributes.value("v").toString();
                        tags[key] = value;
                    }
                } 
            }

            // Filter only ways with highway tag (roads) an complete (2 nodes)
            if (tags.find("highway") == tags.end() || wayNodes.size() < 2) {
                continue;
            }

            // Extract OSM tags (25 tags max for performance)
            auto getTag = [&tags](const QString& key) -> std::optional<QString> {
                auto it = tags.find(key);
                if (it == tags.end() || it->second.isEmpty()) {
                    return std::nullopt;
                }
                return it->second;
            };

            auto highway = getTag("highway");
            auto motorcar = getTag("motorcar");
            auto motorVehicle = getTag("motor_vehicle");
            auto access = getTag("access");
            auto surface = getTag("surface");
            auto bus = getTag("bus");
            auto busLanes = getTag("lanes:bus");
            auto busway = getTag("busway");
            auto publicTransport = getTag("public_transport");
            auto psv = getTag("psv");
            auto psvLanes = getTag("lanes:psv");
            auto bicycle = getTag("bicycle");
            auto cyclewayRight = getTag("cycleway:right");
            auto foot = getTag("foot");
            auto sidewalk = getTag("sidewalk");
            auto footway = getTag("footway");
            auto maxspeed = getTag("maxspeed");
            auto lanes = getTag("lanes");
            auto altName = getTag("alt_name");
            auto officialName = getTag("official_name");
            auto bridge = getTag("bridge");
            auto bridgeName = getTag("bridge:name");
            auto tunnel = getTag("tunnel");
            auto tunnelName = getTag("tunnel:name");
            auto name = getTag("name");

            // Determine oneway
            bool isOneWay = false;
            if (tags.find("oneway") != tags.end()) {
                QString onewayValue = tags["oneway"].toLower();
                if (onewayValue == "yes" || onewayValue == "true" || onewayValue == "1") {
                    isOneWay = true;
                }
            }

            // Process each consecutive pair of nodes in the way
            for (size_t i = 0; i < wayNodes.size() - 1; ++i) {
                int64_t fromId = wayNodes[i];
                int64_t toId = wayNodes[i + 1];
                
                // Verify both nodes exist
                if (nodePositions.find(fromId) == nodePositions.end() ||
                    nodePositions.find(toId) == nodePositions.end()) {
                    continue; // Skip this segment if any node is missing
                }

                auto [fromLat, fromLon] = nodePositions[fromId];
                auto [toLat, toLon] = nodePositions[toId];
                Coordinate fromCoord(fromLat, fromLon);
                Coordinate toCoord(toLat, toLon);

                // Create nodes if not exist
                if (nodes.find(fromId) == nodes.end()) {
                    nodes[fromId] = std::make_shared<Node>(fromId, fromCoord);
                }
                if (nodes.find(toId) == nodes.end()) {
                    nodes[toId] = std::make_shared<Node>(toId, toCoord);
                }

                // Calculate haversine distance
                double weight = fromCoord.distanceTo(toCoord);

                // Convert collected tags (QString -> QString) into unordered_map<string,string>
                std::unordered_map<std::string, std::string> edgeTags;
                edgeTags.reserve(tags.size());
                for (const auto& kv : tags) {
                    if (!kv.first.isEmpty()) {
                        edgeTags.emplace(kv.first.toStdString(), kv.second.toStdString());
                    }
                }

                // Create edge A->B and push to edges vector
                auto edgeAB = std::make_shared<Edge>(
                    nextEdgeId++,
                    nodes[fromId].get(),
                    nodes[toId].get(),
                    isOneWay,
                    Distance(weight),
                    edgeTags
                );
                edges.push_back(edgeAB);

                // If the way is not oneway, create the reverse edge B->A with same tags
                if (!isOneWay) {
                    auto edgeBA = std::make_shared<Edge>(
                        nextEdgeId++,
                        nodes[toId].get(),
                        nodes[fromId].get(),
                        false, // reverse is not one-way
                        Distance(weight),
                        edgeTags
                    );
                    edges.push_back(edgeBA);
                }
            } // End of for loop through way nodes
            
            processedWays++;

            // Report progress every 100 ways
            if (processedWays % 100 == 0 && progressCallback) {
                progressCallback(
                    QString("Procesados %1 ways...").arg(processedWays),
                    0.3 + (0.6 * processedWays / 10000.0) // Estimated
                );
            }
        }
    }

    file.close();

    if (xml.hasError()) {
        throw std::runtime_error("Error parsing OSM XML: " + xml.errorString().toStdString());
    }

    // Debugs
    qDebug() << "OSM parsing completed:";
    qDebug() << " Total ways processed:" << processedWays;
    qDebug() << " Total nodes created:" << nodes.size();
    qDebug() << " Total edges created:" << edges.size();

    if (progressCallback) {
        progressCallback("Finished parsing OSM file...", 1.0);
    }

    // THIRD: Build Graph using addNode and addEdge methods
    auto graph = std::make_shared<Graph>();
    
    // Calculate bounding box while adding nodes
    double minLat = std::numeric_limits<double>::max();
    double maxLat = std::numeric_limits<double>::lowest();
    double minLon = std::numeric_limits<double>::max();
    double maxLon = std::numeric_limits<double>::lowest();
    
    // Add all nodes to graph
    for (const auto& [id, nodePtr] : nodes) {
        const auto& coord = nodePtr->getCoordinate();
        double lat = coord.getLatitude();
        double lon = coord.getLongitude();
        
        graph->addNode(id, lat, lon);
        
        // Update bounding box
        minLat = std::min(minLat, lat);
        maxLat = std::max(maxLat, lat);
        minLon = std::min(minLon, lon);
        maxLon = std::max(maxLon, lon);
    }
    
    // Set bounding box
    if (!nodes.empty()) {
        graph->setBounds(minLat, maxLat, minLon, maxLon);
    }
    
    // Add all edges to graph
    for (const auto& edgePtr : edges) {
        graph->addEdge(
            edgePtr->getId(),
            edgePtr->getSource()->getId(),
            edgePtr->getTarget()->getId(),
            edgePtr->getDistance(),
            edgePtr->IsOneWay(),
            edgePtr->getTags()
        );
    }
    
    // Build adjacency list for pathfinding
    graph->buildAdjacencyList();

    if (progressCallback) {
        progressCallback("Graph built successfully.", 1.0);
    }

    qDebug() << "Final graph statistics:";
    qDebug() << " Nodes in graph:" << graph->getNodeCount();
    qDebug() << " Edges in graph:" << graph->getEdgeCount();

    return graph;
}

}
}