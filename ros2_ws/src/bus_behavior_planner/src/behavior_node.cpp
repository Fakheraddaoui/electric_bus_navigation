#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <std_msgs/msg/string.hpp>
#include "bus_behavior_planner/behavior_fsm.hpp"

using namespace std::chrono_literals;

class BehaviorNode : public rclcpp::Node {
public:
    BehaviorNode() : Node("bus_behavior_planner"),
        fsm_(declare_parameter("cruise_speed", 11.0),
             declare_parameter("comfort_decel", 1.2),
             declare_parameter("min_follow_gap", 8.0))
    {
        cmd_pub_   = create_publisher<geometry_msgs::msg::Twist>("target_motion", 10);
        state_pub_ = create_publisher<std_msgs::msg::String>("behavior_state", 10);
        // Perception inputs (detection fusion node) omitted for brevity;
        // wired via subscriptions to /perception/* in the full launch graph.
        timer_ = create_wall_timer(50ms, [this] { tick(); });
    }

private:
    void tick()
    {
        auto d = fsm_.step(perception_, ego_speed_);
        geometry_msgs::msg::Twist t;
        t.linear.x = d.target_speed;
        cmd_pub_->publish(t);

        std_msgs::msg::String s;
        s.data = std::to_string(static_cast<int>(d.behavior));
        state_pub_->publish(s);
    }

    bus::BehaviorFsm fsm_;
    bus::Perception perception_;
    double ego_speed_{0.0};
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_pub_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr state_pub_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char** argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<BehaviorNode>());
    rclcpp::shutdown();
    return 0;
}
