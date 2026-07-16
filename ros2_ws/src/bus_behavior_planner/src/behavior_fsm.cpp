#include "bus_behavior_planner/behavior_fsm.hpp"
#include <algorithm>
#include <cmath>

namespace bus {

BehaviorFsm::BehaviorFsm(double cruise_speed, double comfort_decel, double min_follow_gap)
    : cruise_speed_(cruise_speed), comfort_decel_(comfort_decel),
      min_follow_gap_(min_follow_gap) {}

double BehaviorFsm::stoppingDistance(double v) const
{
    return (v * v) / (2.0 * comfort_decel_) + 2.0; // + standoff margin
}

Decision BehaviorFsm::step(const Perception& p, double ego_speed)
{
    // Priority 1: imminent obstacle -> emergency stop (safety monitor also
    // watches this path independently on the MCU side).
    if (p.obstacle_in_lane && p.obstacle_distance_m < stoppingDistance(ego_speed)) {
        state_ = Behavior::EMERGENCY_STOP;
        return {state_, 0.0};
    }

    // Priority 2: pedestrian in crosswalk we can still stop for.
    if (p.pedestrian_in_crosswalk &&
        p.pedestrian_distance_m < 1.5 * stoppingDistance(ego_speed)) {
        state_ = Behavior::YIELD_PEDESTRIAN;
        return {state_, 0.0};
    }

    // Priority 3: red/yellow light we can stop for before the line.
    const bool must_stop_light =
        (p.light == LightState::RED ||
         (p.light == LightState::YELLOW &&
          p.light_distance_m > stoppingDistance(ego_speed) * 0.6)) &&
        p.light_distance_m < 2.0 * stoppingDistance(ego_speed);
    if (must_stop_light) {
        state_ = (p.light_distance_m < 3.0 && ego_speed < 0.3)
                 ? Behavior::HOLD : Behavior::STOP_FOR_LIGHT;
        return {state_, 0.0};
    }

    // Priority 4: car-following with time-gap policy.
    if (p.lead_gap_m && *p.lead_gap_m < std::max(min_follow_gap_, 1.8 * ego_speed)) {
        state_ = Behavior::FOLLOW;
        const double lead_v = p.lead_speed.value_or(0.0);
        const double gap_err = *p.lead_gap_m - min_follow_gap_;
        const double v = std::clamp(lead_v + 0.3 * gap_err, 0.0, cruise_speed_);
        return {state_, v};
    }

    state_ = Behavior::CRUISE;
    return {state_, cruise_speed_};
}

} // namespace bus
