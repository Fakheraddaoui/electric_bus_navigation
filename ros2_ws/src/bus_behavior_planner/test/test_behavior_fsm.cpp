#include <gtest/gtest.h>
#include "bus_behavior_planner/behavior_fsm.hpp"

using namespace bus;

class FsmTest : public ::testing::Test {
protected:
    BehaviorFsm fsm{11.0, 1.2, 8.0};
    Perception p;
};

TEST_F(FsmTest, CruisesWithClearRoad)
{
    auto d = fsm.step(p, 10.0);
    EXPECT_EQ(d.behavior, Behavior::CRUISE);
    EXPECT_DOUBLE_EQ(d.target_speed, 11.0);
}

TEST_F(FsmTest, RedLightStops)
{
    p.light = LightState::RED;
    p.light_distance_m = 40.0;
    auto d = fsm.step(p, 10.0);
    EXPECT_EQ(d.behavior, Behavior::STOP_FOR_LIGHT);
    EXPECT_DOUBLE_EQ(d.target_speed, 0.0);
}

TEST_F(FsmTest, RedLightTooFarIsIgnored)
{
    p.light = LightState::RED;
    p.light_distance_m = 500.0;
    EXPECT_EQ(fsm.step(p, 10.0).behavior, Behavior::CRUISE);
}

TEST_F(FsmTest, YellowLightWithRoomStops)
{
    p.light = LightState::YELLOW;
    p.light_distance_m = 60.0;
    EXPECT_EQ(fsm.step(p, 10.0).behavior, Behavior::STOP_FOR_LIGHT);
}

TEST_F(FsmTest, YellowLightTooCloseProceeds)
{
    p.light = LightState::YELLOW;
    p.light_distance_m = 5.0;   // can't stop comfortably: continue
    EXPECT_EQ(fsm.step(p, 12.0).behavior, Behavior::CRUISE);
}

TEST_F(FsmTest, PedestrianOverridesLight)
{
    p.light = LightState::GREEN;
    p.pedestrian_in_crosswalk = true;
    p.pedestrian_distance_m = 20.0;
    auto d = fsm.step(p, 8.0);
    EXPECT_EQ(d.behavior, Behavior::YIELD_PEDESTRIAN);
    EXPECT_DOUBLE_EQ(d.target_speed, 0.0);
}

TEST_F(FsmTest, CutInTriggersEmergencyStop)
{
    p.obstacle_in_lane = true;
    p.obstacle_distance_m = 10.0;
    EXPECT_EQ(fsm.step(p, 12.0).behavior, Behavior::EMERGENCY_STOP);
}

TEST_F(FsmTest, EmergencyBeatsPedestrianBeatsLight)
{
    p.obstacle_in_lane = true;  p.obstacle_distance_m = 5.0;
    p.pedestrian_in_crosswalk = true; p.pedestrian_distance_m = 5.0;
    p.light = LightState::RED;  p.light_distance_m = 5.0;
    EXPECT_EQ(fsm.step(p, 10.0).behavior, Behavior::EMERGENCY_STOP);
}

TEST_F(FsmTest, FollowsSlowLeadVehicle)
{
    p.lead_gap_m = 12.0;
    p.lead_speed = 6.0;
    auto d = fsm.step(p, 10.0);
    EXPECT_EQ(d.behavior, Behavior::FOLLOW);
    EXPECT_LT(d.target_speed, 11.0);
}

TEST_F(FsmTest, HoldsAtLineWhenStopped)
{
    p.light = LightState::RED;
    p.light_distance_m = 2.0;
    EXPECT_EQ(fsm.step(p, 0.1).behavior, Behavior::HOLD);
}

TEST_F(FsmTest, StoppingDistanceGrowsQuadratically)
{
    EXPECT_GT(fsm.stoppingDistance(20.0), 3.9 * fsm.stoppingDistance(10.0) - 8.0);
}
