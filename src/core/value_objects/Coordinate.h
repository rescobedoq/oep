#pragma once

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

    // Haversine distance in meters
    double distanceTo(const Coordinate& other) const {
        const double R = 6371e3; // Earth radius in meters
        double phi1 = latitude * PI / 180.0;
        double phi2 = other.getLatitude() * PI / 180.0;
        double deltaPhi = (other.getLatitude() - latitude) * PI / 180.0;
        double deltaLambda = (other.getLongitude() - longitude) * PI / 180.0;

        double a = sin(deltaPhi / 2) * sin(deltaPhi / 2) +
                   cos(phi1) * cos(phi2) *
                   sin(deltaLambda / 2) * sin(deltaLambda / 2);

        double c = 2 * atan2(sqrt(a), sqrt(1 - a));

        return R * c; // Distance in meters
    }

    // Manhattan distance in meters (For A*)
    double manhattanDistanceTo(const Coordinate& other) const {
        return std::abs(latitude - other.getLatitude()) + 
               std::abs(longitude - other.getLongitude());
    }

    // Comparison operators
    bool operator==(const Coordinate& other) const {
        return latitude == other.getLatitude() && longitude == other.getLongitude();
    }
    bool operator!=(const Coordinate& other) const {
        return !(*this == other);
    }
};