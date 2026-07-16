from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        Node(package='bus_behavior_planner', executable='behavior_node',
             parameters=[{'cruise_speed': 11.0, 'comfort_decel': 1.2}]),
        Node(package='bus_diagnostics', executable='diag_aggregator'),
    ])
