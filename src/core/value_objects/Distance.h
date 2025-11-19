#include <stdexcept>

class Distance {
private:
    double meters;

public:
    Distance(double meters) : meters(meters) {
        if (meters < 0) {
            throw std::invalid_argument("Distance cannot be negative");
        }
    }
    
    // Getters
    double getMeters() const { return meters; }
    double getKilometers() const { return meters / 1000.0; }
    double getMiles() const { return meters / 1609.34; } 

    // Comparison operators
    Distance operator+(const Distance& other) const {
        return Distance(meters + other.getMeters());
    }
    Distance operator+=(const Distance& other) {
        meters += other.getMeters();
        return *this;
    }
    Distance operator-(const Distance& other) const {
        double result = meters - other.getMeters();
        return Distance(result < 0 ? 0 : result);
    }
    Distance operator-=(const Distance& other) {
        meters -= other.getMeters();
        return *this;
    }
    bool operator<(const Distance& other) const {
        return meters < other.getMeters();
    }
    bool operator>(const Distance& other) const {
        return meters > other.getMeters();
    }
    bool operator==(const Distance& other) const {
        return meters == other.getMeters();
    }
    bool operator!=(const Distance& other) const {
        return !(*this == other);
    }
};