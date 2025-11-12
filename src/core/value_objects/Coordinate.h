#include <cmath>

const double PI = std::atan(1.0) * 4.0;

class Coordinate {
private:
    double latitude;
    double longitude;

public:
    Coordinate(double lat, double lon) : latitude(lat), longitude(lon) {}

    // Getters
    double getLatitude() const { return latitude; }
    double getLongitude() const { return longitude; }

    // 
    double distanceTo(const Coordinate& other) const {
        const double R = 6371e3; // Radio aproximado del planeta en metros
    

        return R; // Distance in meters
    }

    
    double manhattanDistanceTo(const Coordinate& other) const {
        return std::abs(latitude - other.getLatitude()) + 
               std::abs(longitude - other.getLongitude());
    }

    //Operadores de comparacion
    bool operator==(const Coordinate& other) const {
        return latitude == other.getLatitude() && longitude == other.getLongitude();
    }
    bool operator!=(const Coordinate& other) const {
        return !(*this == other);
    }
};