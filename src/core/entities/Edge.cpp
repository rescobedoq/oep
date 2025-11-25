#include "Edge.h"

Edge::Edge(
    int64_t id,
    Node* source,
    Node* target,
    bool isOneWay,
    const Distance& distance,
    const std::unordered_map<std::string, std::string>& tags
)
    : id(id), source(source), target(target), isOneWay(isOneWay), distance(distance), tags(tags) {}

void Edge::addTag(const std::string& key, const std::string& value) { tags[key] = value; }

std::string Edge::getTag(const std::string& key) const { return tags.at(key); }

bool Edge::hasTag(const std::string& key) const { return tags.find(key) != tags.end(); }

std::string Edge::getStreetName() const {
    // Try to get name tag
    auto it = tags.find("name");
    if (it != tags.end() && !it->second.empty()) {
        return it->second;
    }
    
    // Fallback: Generate name based on highway type
    auto hwIt = tags.find("highway");
    if (hwIt != tags.end() && !hwIt->second.empty()) {
        const std::string& highway = hwIt->second;
        
        // Map highway types to Spanish names (matching Java implementation)
        if (highway == "residential") return "Camino Residencial";
        if (highway == "footway") return "Senda";
        if (highway == "primary") return "Vía Principal";
        if (highway == "secondary") return "Vía Secundaria";
        if (highway == "tertiary") return "Vía Terciaria";
        if (highway == "trunk") return "Vía Troncal";
        if (highway == "motorway") return "Autopista";
        if (highway == "living_street") return "Calle Residencial";
        if (highway == "pedestrian") return "Zona Peatonal";
        if (highway == "cycleway") return "Ciclovía";
        if (highway == "path") return "Sendero";
        if (highway == "track") return "Camino Rural";
        if (highway == "service") return "Vía de Servicio";
        if (highway == "unclassified") return "Camino Sin Clasificar";
        if (highway == "steps") return "Escalones";
        if (highway == "bridleway") return "Camino de Herradura";
        if (highway == "tertiary_link") return "Enlace Terciario";
        if (highway == "primary_link") return "Enlace Principal";
        if (highway == "secondary_link") return "Enlace Secundario";
        if (highway == "trunk_link") return "Enlace Troncal";
        if (highway == "corridor") return "Pasillo";
        if (highway == "raceway") return "Pista de Carreras";
        if (highway == "construction") return "En Construcción";
        
        return "Vía " + highway;
    }
    
    return "Vía sin nombre";
}