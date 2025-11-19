#include <string>
#include <unordered_map>
#include "../value_objects/Coordinate.h"

class Node {
private:
    int64_t id;
    Coordinate coordinate;      // Con latitud y longitud

public:
    Node(
        int64_t id, 
        const Coordinate& coordinate 
    );
    
    //Getters
    int64_t getId() const { return id; }
    Coordinate& getCoordinate() { return coordinate; }

    // Comparation
    bool operator==(const Node& other) const { return id == other.getId(); }
    bool operator!=(const Node& other) const { return id != other.getId(); }
};