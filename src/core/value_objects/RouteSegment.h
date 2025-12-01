#pragma once

#include "../entities/Node.h"
#include "../entities/Edge.h"
#include <string>
#include "Distance.h"

class RouteSegment {
private:
    Node* source;
    Node* target;
    Edge* edge;
    double durationSeconds;
    std::string roadName;        // "Av. Principal"
    std::string instruction;     // "Continue straight for Av. Principal"

public:
    RouteSegment(
        Node* source, 
        Node* target, 
        Edge* edge, 
        double durationSeconds,
        const std::string& roadName = "",
        const std::string& instruction = ""
    )
        : source(source), target(target), edge(edge), durationSeconds(durationSeconds),
        roadName(roadName), instruction(instruction) {}

    // Getters
    Node* getSource() const { return source; }
    Node* getTarget() const { return target; }
    Edge* getEdge() const { return edge; }
    double getDurationSeconds() const { return durationSeconds; }
    std::string getRoadName() const { return roadName; }
    std::string getInstruction() const { return instruction; }
};