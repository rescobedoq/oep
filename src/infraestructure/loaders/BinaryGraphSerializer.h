#include <QString>
#include <memory>
#include "../../core/entities/Graph.h"

namespace services {
namespace io {

class BinaryGraphSerializer {
public:
    // Serialize the graph to a binary file
    static void serialize(
        const std::shared_ptr<Graph>& graph,
        const QString& filePath
    );

private:
    // Header structure (128 bytes total for alignment)
    struct BinaryHeader {
        char magic[8];          // "OGRGRAPH"
        int32_t version;        // 4 bytes
        int64_t nodeCount;      // 8 bytes
        int64_t edgeCount;      // 8 bytes
        double minLatitude;     // 8 bytes
        double maxLatitude;     // 8 bytes
        double minLongitude;    // 8 bytes
        double maxLongitude;    // 8 bytes
        char padding[72];       // Padding to 128 bytes total
    };
};

}
}