#include "../../core/entities/Graph.h"
#include <QString>
#include <memory>
#include <functional>

namespace services {
namespace io {

class OSMGraphLoader {
public:
    using ProgressCallback = std::function<void(const QString&, double)>;

    static std::shared_ptr<Graph> load(
        const QString& filePath,
        ProgressCallback progressCallback = nullptr
    );
};

}
}