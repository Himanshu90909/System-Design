#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace uber {

struct Location {
    double latitude;
    double longitude;
};

struct Driver {
    std::string id;
    Location location;
    std::string vehicleType;
    bool available{true};
    bool childSeat{false};
    bool wheelchairAccessible{false};
    std::uint64_t locationVersion{0};
};

enum class RideState { Requested, DriverAssigned, DriverArrived, InProgress, Completed, Cancelled };

struct RideRequest {
    std::string id;
    std::string riderId;
    Location pickup;
    Location destination;
    std::string vehicleType;
    RideState state{RideState::Requested};
    std::optional<std::string> driverId;
    double estimatedFare{0.0};
};

class DispatchService {
    mutable std::mutex mutex_;
    std::unordered_map<std::string, Driver> drivers_;
    std::unordered_map<std::string, RideRequest> rides_;
    std::uint64_t nextRide_{1};

    static double distance(Location a, Location b) {
        const double lat = (a.latitude - b.latitude) * 111.0;
        const double lon = (a.longitude - b.longitude) * 111.0 * std::cos(a.latitude * 3.141592653589793 / 180.0);
        return std::sqrt(lat * lat + lon * lon);
    }

    static std::string cell(Location location) {
        return std::to_string(static_cast<int>(std::floor(location.latitude * 100))) + ":" +
               std::to_string(static_cast<int>(std::floor(location.longitude * 100)));
    }

    Driver& driver(const std::string& id) {
        auto it = drivers_.find(id);
        if (it == drivers_.end()) throw std::out_of_range("driver not found");
        return it->second;
    }

    RideRequest& ride(const std::string& id) {
        auto it = rides_.find(id);
        if (it == rides_.end()) throw std::out_of_range("ride not found");
        return it->second;
    }

public:
    void registerDriver(Driver driverToAdd) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (driverToAdd.id.empty()) throw std::invalid_argument("driver id is required");
        drivers_[driverToAdd.id] = std::move(driverToAdd);
    }

    void updateLocation(const std::string& driverId, Location location) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto& current = driver(driverId);
        current.location = location;
        ++current.locationVersion;
    }

    std::string locationCell(const std::string& driverId) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = drivers_.find(driverId);
        if (it == drivers_.end()) throw std::out_of_range("driver not found");
        return cell(it->second.location);
    }

    std::string requestRide(std::string riderId, Location pickup, Location destination, std::string vehicleType) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (riderId.empty() || vehicleType.empty()) throw std::invalid_argument("rider and vehicle type are required");
        const auto id = "ride-" + std::to_string(nextRide_++);
        rides_.emplace(id, RideRequest{id, std::move(riderId), pickup, destination, std::move(vehicleType)});
        return id;
    }

    std::optional<std::string> dispatch(const std::string& rideId, double radiusKm = 5.0) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto& requestedRide = ride(rideId);
        if (requestedRide.state != RideState::Requested) throw std::runtime_error("ride is not requestable");
        std::string selected;
        double bestDistance = radiusKm;
        for (const auto& entry : drivers_) {
            const auto& candidate = entry.second;
            if (!candidate.available || candidate.vehicleType != requestedRide.vehicleType) continue;
            const auto candidateDistance = distance(candidate.location, requestedRide.pickup);
            if (candidateDistance <= bestDistance) { bestDistance = candidateDistance; selected = candidate.id; }
        }
        if (selected.empty()) return std::nullopt;
        auto& assigned = driver(selected);
        assigned.available = false;
        requestedRide.driverId = selected;
        requestedRide.state = RideState::DriverAssigned;
        requestedRide.estimatedFare = 3.0 + distance(requestedRide.pickup, requestedRide.destination) * 2.25;
        return selected;
    }

    void markArrived(const std::string& rideId) { transition(rideId, RideState::DriverAssigned, RideState::DriverArrived); }
    void startTrip(const std::string& rideId) { transition(rideId, RideState::DriverArrived, RideState::InProgress); }

    void completeTrip(const std::string& rideId) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto& current = ride(rideId);
        if (current.state != RideState::InProgress) throw std::runtime_error("ride is not in progress");
        current.state = RideState::Completed;
        if (current.driverId) driver(*current.driverId).available = true;
    }

    void cancelRide(const std::string& rideId) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto& current = ride(rideId);
        if (current.state == RideState::Completed || current.state == RideState::Cancelled) throw std::runtime_error("ride cannot be cancelled");
        current.state = RideState::Cancelled;
        if (current.driverId) driver(*current.driverId).available = true;
    }

    RideRequest getRide(const std::string& rideId) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = rides_.find(rideId);
        if (it == rides_.end()) throw std::out_of_range("ride not found");
        return it->second;
    }

private:
    void transition(const std::string& rideId, RideState expected, RideState next) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto& current = ride(rideId);
        if (current.state != expected) throw std::runtime_error("invalid ride state transition");
        current.state = next;
    }
};

} // namespace uber

int main() {
    using namespace uber;
    DispatchService service;
    service.registerDriver({"driver-near", {28.6139, 77.2090}, "SEDAN"});
    service.registerDriver({"driver-far", {28.7000, 77.3000}, "SEDAN"});
    service.updateLocation("driver-near", {28.6140, 77.2091});

    const auto rideId = service.requestRide("rider-1", {28.6138, 77.2092}, {28.6300, 77.2200}, "SEDAN");
    const auto selected = service.dispatch(rideId);
    if (!selected || *selected != "driver-near") return 1;
    service.markArrived(rideId);
    service.startTrip(rideId);
    service.completeTrip(rideId);

    const auto ride = service.getRide(rideId);
    if (ride.state != RideState::Completed || ride.estimatedFare <= 3.0) return 1;
    std::cout << "Uber dispatch checks passed\n";
    std::cout << "Ride: " << ride.id << " | Driver: " << *ride.driverId << " | Fare: $"
              << std::fixed << std::setprecision(2) << ride.estimatedFare << "\n";
}
