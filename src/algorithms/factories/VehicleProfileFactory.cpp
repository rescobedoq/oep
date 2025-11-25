#include "VehicleProfileFactory.h"
#include <algorithm>

std::unique_ptr<VehicleProfile> VehicleProfileFactory::createCarProfile() {
    auto car = std::make_unique<VehicleProfile>("Auto", "CAR");

    car->setSpeed(80.0); // km/h

    // Preferred routes types - speedFactor >= 1.0
    car->setSpeedFactor("residential", 1.0);
    car->setSpeedFactor("primary", 1.3);
    car->setSpeedFactor("secondary", 1.2);
    car->setSpeedFactor("tertiary", 1.1);
    car->setSpeedFactor("trunk", 1.4);
    car->setSpeedFactor("motorway", 1.5);
    car->setSpeedFactor("unclassified", 0.9);
    car->setSpeedFactor("tertiary_link", 1.1);
    car->setSpeedFactor("primary_link", 1.3);
    car->setSpeedFactor("secondary_link", 1.2);
    car->setSpeedFactor("trunk_link", 1.4);
    car->setSpeedFactor("corridor", 0.8);
    
    // Avoided route types - 0.0 < speedFactor < 1.0
    car->setSpeedFactor("track", 0.3);

    // Blocked route types - speedFactor = 0.0
    car->setSpeedFactor("footway", 0.0);
    car->setSpeedFactor("pedestrian", 0.0);
    car->setSpeedFactor("cycleway", 0.0);
    car->setSpeedFactor("path", 0.0);
    car->setSpeedFactor("service", 0.0);
    car->setSpeedFactor("steps", 0.0);
    car->setSpeedFactor("bridleway", 0.0);
    car->setSpeedFactor("living_street", 0.0);
    car->setSpeedFactor("raceway", 0.0);
    car->setSpeedFactor("construction", 0.0);

    return car;
}

std::unique_ptr<VehicleProfile> VehicleProfileFactory::createPedestrianProfile() {
    auto pedestrian = std::make_unique<VehicleProfile>("Peaton", "PEDESTRIAN");

    pedestrian->setSpeed(5.0); // km/h

    // Preferred routes types - speedFactor >= 1.0
    pedestrian->setSpeedFactor("footway", 1.5);
    pedestrian->setSpeedFactor("pedestrian", 1.4);
    pedestrian->setSpeedFactor("cycleway", 1.2);
    pedestrian->setSpeedFactor("path", 1.6);
    pedestrian->setSpeedFactor("service", 1.1);
    pedestrian->setSpeedFactor("steps", 0.8);
    pedestrian->setSpeedFactor("bridleway", 1.3);
    pedestrian->setSpeedFactor("construction", 0.9);
    pedestrian->setSpeedFactor("residential", 1.0);
    
    // Avoided route types - 0.0 < speedFactor < 1.0
    pedestrian->setSpeedFactor("primary", 0.4);
    pedestrian->setSpeedFactor("secondary", 0.5);
    pedestrian->setSpeedFactor("tertiary", 0.7);
    pedestrian->setSpeedFactor("unclassified", 0.6);
    pedestrian->setSpeedFactor("track", 1.1);
    pedestrian->setSpeedFactor("corridor", 0.8);
    
    // Blocked route types - speedFactor = 0.0
    pedestrian->setSpeedFactor("trunk", 0.0);
    pedestrian->setSpeedFactor("motorway", 0.0);
    pedestrian->setSpeedFactor("living_street", 0.0);
    pedestrian->setSpeedFactor("raceway", 0.0);
    pedestrian->setSpeedFactor("tertiary_link", 0.0);
    pedestrian->setSpeedFactor("primary_link", 0.0);
    pedestrian->setSpeedFactor("secondary_link", 0.0);
    pedestrian->setSpeedFactor("trunk_link", 0.0);

    return pedestrian;
}

std::unique_ptr<VehicleProfile> VehicleProfileFactory::getProfile(const std::string& profileName) {
    if (profileName == "car" || profileName == "CAR") {
        return VehicleProfileFactory::createCarProfile();
    } else if (profileName == "peaton" || profileName == "PEDESTRIAN") {
        return VehicleProfileFactory::createPedestrianProfile();
    } else {
        throw std::invalid_argument("Vehicle profile not supported: " + profileName);
    }
}