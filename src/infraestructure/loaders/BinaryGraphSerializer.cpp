#include "BinaryGraphSerializer.h"
#include <QFile>
#include <QDataStream>
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <set>
#include <map>
#include <cstring>

namespace services {
namespace io {

void BinaryGraphSerializer::serialize(
    const std::shared_ptr<Graph>& graph,
    const QString& filePath
) {
    // Create directory if it doesn't exist
    QFileInfo fileInfo(filePath);
    QDir dir = fileInfo.absoluteDir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        throw std::runtime_error("Could not open file for writing: " + filePath.toStdString());
    }

    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);

    // Get const references to internal maps (no copy)
    const auto& nodes = graph->getNodesMap();
    const auto& edges = graph->getEdgesMap();
    
    // Extract bounding box (tuple unpacking)
    auto [minLat, maxLat, minLon, maxLon] = graph->getBounds();

    qDebug() << "Serializing graph to binary file:" << filePath;
    qDebug() << " Nodes:" << nodes.size() << ", Edges:" << edges.size();
    qDebug() << " Bounding Box: [" << minLat << "," << maxLat << "] x [" << minLon << "," << maxLon << "]";

    // FIRST: Build string table (space optimization)
    std::set<QString> uniqueStrings;

    for (const auto& [id, edge] : edges) {
        for (const auto& [key, value] : edge->getTags()) {
            uniqueStrings.insert(QString::fromStdString(key));
            uniqueStrings.insert(QString::fromStdString(value));
        }
    }

    // Asign ids to strings
    std::map<QString, int32_t> stringToId;
    int32_t nextId = 0;
    for (const auto& str : uniqueStrings) {
        stringToId[str] = nextId++;
    }

    qDebug() << " Unique strings:" << uniqueStrings.size();

    // SECOND: Write header
    BinaryHeader header;
    std::memcpy(header.magic, "OGRGRAPH", 8);
    header.version = 1;
    header.nodeCount = static_cast<int64_t>(nodes.size());
    header.edgeCount = static_cast<int64_t>(edges.size());
    header.minLatitude = minLat;
    header.maxLatitude = maxLat;
    header.minLongitude = minLon;
    header.maxLongitude = maxLon;
    std::memset(header.padding, 0, 72);

    stream.writeRawData(header.magic, 8);
    stream << header.version;
    stream << header.nodeCount;
    stream << header.edgeCount;
    stream << header.minLatitude;
    stream << header.maxLatitude;
    stream << header.minLongitude;
    stream << header.maxLongitude;
    stream.writeRawData(header.padding, 72);

    // THIRD: Write string table
    int32_t stringCount = static_cast<int32_t>(uniqueStrings.size());
    stream << stringCount;

    for (const auto& [str, id] : stringToId) {
        QByteArray utf8 = str.toUtf8();
        int32_t length = static_cast<int32_t>(utf8.size());

        stream << id;
        stream << length;
        stream.writeRawData(utf8.constData(), length);
    }

    // FOURTH: Write nodes
    for (const auto& [id, node] : nodes) {
        stream << id;
        stream << node->getCoordinate().getLatitude();
        stream << node->getCoordinate().getLongitude();
    }

    // FIFTH: Write edges
    auto getStringId = [&stringToId](const std::optional<QString>& optStr) -> int32_t {
        if (!optStr) return -1;     // Null string
        auto it = stringToId.find(*optStr);
        if (it == stringToId.end()) return -1;  // Not found
        return it->second;
    };

    char edgePadding[7] = {0};

    for (const auto& [id, edge] : edges) {
        stream << id;
        stream << edge->getSource()->getId();
        stream << edge->getTarget()->getId();
        stream << static_cast<qint8>(edge->IsOneWay() ? 1 : 0);
        stream << edge->getDistance().getMeters();
        stream.writeRawData(edgePadding, 7);

        // Write tags
        auto& tags = edge->getTags();
        int32_t tagCount = static_cast<int32_t>(tags.size());
        stream << tagCount;

        for (const auto& [key, value] : tags) {
            int32_t keyId = getStringId(QString::fromStdString(key));
            int32_t valueId = getStringId(QString::fromStdString(value));
            stream << keyId;
            stream << valueId;
        }
    }

    file.close();

    qDebug() << "Graph serialization completed:" << filePath;
}

}
}
