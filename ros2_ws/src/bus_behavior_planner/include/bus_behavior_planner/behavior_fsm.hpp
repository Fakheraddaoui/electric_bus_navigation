#pragma once
#include <optional>
#include <vector>
#include <cstdint>

namespace bus {

enum class Behavior { CRUISE, FOLLOW, STOP_FOR_LIGHT, YIELD_PEDESTRIAN, EMERGENCY_STOP, HOLD };

enum class LightState { UNKNOWN, GREEN, YELLOW, RED };

struct Perception {
    LightState light{LightState::UNKNOWN};
    double light_distance_m{1e9};       // distance to stop line
    std::optional<double> lead_gap_m;   // gap to lead vehicle
    std::optional<double> lead_speed;   // lead vehicle speed m/s
    bool pedestrian_in_crosswalk{false};
    double pedestrian_distance_m{1e9};
    bool obstacle_in_lane{false};       // sudden cut-in / debris
    double obstacle_distance_m{1e9};
};

struct Decision {
    Behavior behavior;
    double target_speed;   // m/s
};

class BehaviorFsm {
public:
    explicit BehaviorFsm(double cruise_speed = 11.0,   // ~40 km/h
                         double comfort_decel = 1.2,
                         double min_follow_gap = 8.0);

    Decision step(const Perception& p, double ego_speed);
    Behavior current() const { return state_; }

    // Distance needed to stop comfortably from speed v
    double stoppingDistance(double v) const;

private:
    double cruise_speed_, comfort_decel_, min_follow_gap_;
    Behavior state_{Behavior::CRUISE};
};

} // namespace bus
