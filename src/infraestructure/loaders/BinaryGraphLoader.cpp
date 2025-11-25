#include "BinaryGraphLoader.h"
#include <QFile>
#include <QDataStream>
#include <QDebug>
#include <map>
#include <vector>
#include <stdexcept>
#include <cstring>

namespace services {
namespace io {

std::shared_ptr<Graph> BinaryGraphLoader::load(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        throw std::runtime_error("Could not open binary graph file: " + filePath.toStdString());
    }

    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);

    // FIRST: Read header
    BinaryHeader header;
    stream.readRawData(header.magic, 8);

    // Verify magic number
    if (std::string(header.magic, 8) != "OGRGRAPH") {
        throw std::runtime_error("Invalid binary graph file (bad magic): " + filePath.toStdString());
    }

    stream >> header.version;
    stream >> header.nodeCount;
    stream >> header.edgeCount;
    stream >> header.minLatitude;
    stream >> header.maxLatitude;
    stream >> header.minLongitude;
    stream >> header.maxLongitude;
    stream.readRawData(header.padding, 72);

    qDebug() << "Loading graph from binary file:" << filePath;
    qDebug() << " Version:" << header.version;
    qDebug() << " Nodes:" << header.nodeCount << ", Edges:" << header.edgeCount;
    qDebug() << " Bounding Box: [" << header.minLatitude << "," << header.maxLatitude 
             << "] x [" << header.minLongitude << "," << header.maxLongitude << "]";

    // SECOND: Read string table
    int32_t stringCount;
    stream >> stringCount;

    std::map<int32_t, QString> stringTable;

    for (int32_t i = 0; i < stringCount; ++i) {
        int32_t id, length;
        stream >> id >> length;

        QByteArray utf8Bytes(length, 0);
        stream.readRawData(utf8Bytes.data(), length);
        
        QString str = QString::fromUtf8(utf8Bytes);
        stringTable[id] = str;
    }

    qDebug() << " Unique strings loaded:" << stringCount;

    // THIRD: Read nodes and create Graph
    auto graph = std::make_shared<Graph>();

    for (int64_t i = 0; i < header.nodeCount; ++i) {
        int64_t nodeId;
        double lat, lon;

        stream >> nodeId >> lat >> lon;

        graph->addNode(nodeId, lat, lon);
    }

    // Set bounding box
    graph->setBounds(
        header.minLatitude,
        header.maxLatitude,
        header.minLongitude,
        header.maxLongitude
    );

    qDebug() << " Nodes loaded:" << graph->getNodeCount();

    // FOURTH: Read edges
    // Helper function to get string from table
    auto getStringFromTable = [&stringTable](int32_t id) -> std::string {
        if (id == -1) return "";
        auto it = stringTable.find(id);
        if (it == stringTable.end()) return "";
        return it->second.toStdString();
    };

    char edgePadding[7];

    for (int64_t i = 0; i < header.edgeCount; ++i) {
        // Read fixed data
        int64_t id, fromId, toId;
        double meters;
        qint8 onewayByte;

        stream >> id >> fromId >> toId >> onewayByte >> meters;
        stream.readRawData(edgePadding, 7);

        bool isOneWay = (onewayByte != 0);

        // Read tag count and tags
        int32_t tagCount;
        stream >> tagCount;

        std::unordered_map<std::string, std::string> tags;
        tags.reserve(tagCount);

        for (int32_t j = 0; j < tagCount; ++j) {
            int32_t keyId, valueId;
            stream >> keyId >> valueId;

            std::string key = getStringFromTable(keyId);
            std::string value = getStringFromTable(valueId);

            if (!key.empty()) {
                tags[key] = value;
            }
        }

        // Add edge to graph
        graph->addEdge(
            id,
            fromId,
            toId,
            Distance(meters),
            isOneWay,
            tags
        );
    }

    qDebug() << " Edges loaded:" << graph->getEdgeCount();

    file.close();

    // Build adjacency list
    graph->buildAdjacencyList();

    qDebug() << "Binary graph loaded successfully";

    return graph;
}

}
}