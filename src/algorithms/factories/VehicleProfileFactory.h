#include "../VehicleProfile.h"
#include <string>
#include <memory>
#include <stdexcept>

class VehicleProfileFactory {
public:
    static std::unique_ptr<VehicleProfile> createCarProfile();

    static std::unique_ptr<VehicleProfile> createPedestrianProfile();

    static std::unique_ptr<VehicleProfile> getProfile(const std::string& profileName);

    static std::unique_ptr<VehicleProfile> createProfile(const std::string& profileName) {
        return getProfile(profileName);
    }

    static std::vector<std::string> getAvailableProfiles() {
        return {"CAR", "PEDESTRIAN"};
    }
};