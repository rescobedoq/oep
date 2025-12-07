#include <QString>
#include <memory>
#include "../../core/entities/Graph.h"

namespace services {
namespace io {

class BinaryGraphLoader {
public:
    // Load the graph from a binary file
    static std::shared_ptr<Graph> load(const QString& filePath);

private:
    // Header structure
    struct BinaryHeader {
        char magic[8];      // "OGRGRAPH"
        int32_t version;  // Version number
        int64_t nodeCount;
        int64_t edgeCount;
        double minLatitude;
        double maxLatitude;
        double minLongitude;
        double maxLongitude;
        char padding[72];
    };
};

}
}