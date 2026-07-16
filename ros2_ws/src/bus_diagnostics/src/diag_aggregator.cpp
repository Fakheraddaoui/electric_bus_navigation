#include <rclcpp/rclcpp.hpp>
#include <diagnostic_msgs/msg/diagnostic_array.hpp>
#include <unordered_map>

using namespace std::chrono_literals;
using diagnostic_msgs::msg::DiagnosticArray;
using diagnostic_msgs::msg::DiagnosticStatus;

/*
 * Collects DiagnosticStatus from every node (Jetsons + both micro-ROS MCUs:
 * cpu load, task timing, error counters, firmware version/self-test) and
 * publishes a fleet-style rollup plus a single worst-of health byte that the
 * safety monitor also consumes.
 */
class DiagAggregator : public rclcpp::Node {
public:
    DiagAggregator() : Node("bus_diag_aggregator")
    {
        sub_ = create_subscription<DiagnosticArray>(
            "/diagnostics", 50, [this](DiagnosticArray::SharedPtr msg) {
                for (auto& s : msg->status) latest_[s.name] = s;
            });
        pub_ = create_publisher<DiagnosticArray>("/diagnostics_agg", 10);
        timer_ = create_wall_timer(1s, [this] { publish(); });
    }

private:
    void publish()
    {
        DiagnosticArray agg;
        agg.header.stamp = now();
        uint8_t worst = DiagnosticStatus::OK;
        for (auto& [name, st] : latest_) {
            agg.status.push_back(st);
            worst = std::max(worst, st.level);
        }
        DiagnosticStatus rollup;
        rollup.name = "bus/system_health";
        rollup.level = worst;
        rollup.message = worst == DiagnosticStatus::OK ? "all systems nominal"
                                                       : "degraded — see components";
        agg.status.push_back(rollup);
        pub_->publish(agg);
    }

    std::unordered_map<std::string, DiagnosticStatus> latest_;
    rclcpp::Subscription<DiagnosticArray>::SharedPtr sub_;
    rclcpp::Publisher<DiagnosticArray>::SharedPtr pub_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char** argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<DiagAggregator>());
    rclcpp::shutdown();
    return 0;
}
