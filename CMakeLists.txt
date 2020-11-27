#  ----------------------------------------------------------------------------
#  2020 Bernd Pfrommer bernd.pfrommer@gmail.com
#
cmake_minimum_required(VERSION 3.5)
project(flir_spinnaker_common)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

# the spinnaker SDK does not provide a cmake file
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
find_package(Spinnaker REQUIRED)

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(ament_cmake_auto REQUIRED)
find_package(ament_cmake_ros REQUIRED)

add_library(flir_spinnaker_common  src/driver.cpp src/driver_impl.cpp)
target_link_libraries(flir_spinnaker_common PRIVATE Spinnaker::Spinnaker)

target_include_directories(flir_spinnaker_common PUBLIC  
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>  
    $<INSTALL_INTERFACE:include/flir_spinnaker_common>
    )

install(TARGETS flir_spinnaker_common
  EXPORT export_flir_spinnaker_common
  LIBRARY  DESTINATION lib/${PROJECT_NAME}
  PUBLIC_HEADER DESTINATION  include/${PROJECT_NAME}
  INCLUDES DESTINATION ${CMAKE_INSTALL_PREFIX}/include
  )

install(
  DIRECTORY include
  DESTINATION ${CMAKE_INSTALL_PREFIX}
)

ament_export_targets(export_flir_spinnaker_common)

ament_package()

