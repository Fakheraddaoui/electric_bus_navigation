cmake_minimum_required(VERSION 3.8)
project(bus_behavior_planner)
set(CMAKE_CXX_STANDARD 17)
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(std_msgs REQUIRED)

add_library(behavior src/behavior_fsm.cpp)
target_include_directories(behavior PUBLIC include)

add_executable(behavior_node src/behavior_node.cpp)
target_link_libraries(behavior_node behavior)
ament_target_dependencies(behavior_node rclcpp geometry_msgs std_msgs)

install(TARGETS behavior_node DESTINATION lib/${PROJECT_NAME})
install(DIRECTORY launch DESTINATION share/${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)
  ament_add_gtest(test_behavior_fsm test/test_behavior_fsm.cpp)
  target_link_libraries(test_behavior_fsm behavior)
endif()
ament_package()
