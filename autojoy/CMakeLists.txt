cmake_minimum_required(VERSION 2.8.3)
project(autojoy)

## Compile as C++11, supported in ROS Kinetic and newer
# add_compile_options(-std=c++11)

## Find catkin macros and libraries
## if COMPONENTS list like find_package(catkin REQUIRED COMPONENTS xyz)
## is used, also find other catkin packages

find_package(catkin REQUIRED COMPONENTS
  message_generation
  roscpp
  std_msgs
  darknet_ros
  darknet_ros_msgs
  sensor_msgs
)

add_message_files(FILES DetectMsg.msg FILES LidarMsg.msg FILES ControlMsg.msg)
generate_messages(DEPENDENCIES std_msgs)


find_package(Boost REQUIRED)

###################################
## catkin specific configuration ##
###################################

catkin_package(
  LIBRARIES autojoy
  CATKIN_DEPENDS  roscpp std_msgs darknet_ros darknet_ros_msgs sensor_msgs
  DEPENDS Boost
)

###########
## Build ##
###########

link_directories(/usr/local/cuda/lib64)

include_directories(
  include
  ${catkin_INCLUDE_DIRS}
  ${catkin_INCLUDE_DIRS}/include
  ${catkin_INCLUDE_DIRS}/src
)

add_executable(car_sub src/carDetect.cpp)
add_dependencies(car_sub ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(car_sub cuda cudart cublas ${catkin_LIBRARIES})

add_executable(Lidar_sub src/LidarDetect.cpp)
add_dependencies(Lidar_sub ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(Lidar_sub cuda cudart cublas ${catkin_LIBRARIES})

add_executable(carControl src/carControl.cpp)
add_dependencies(carControl ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(carControl cuda cudart cublas ${catkin_LIBRARIES})

add_executable(MotorControl src/motorControl.cpp)
add_dependencies(MotorControl ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(MotorControl cuda cudart cublas ${catkin_LIBRARIES})

#############
## Install ##
#############
