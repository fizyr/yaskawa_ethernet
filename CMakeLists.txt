cmake_minimum_required(VERSION 3.8)
project(yaskawa_ethernet)

option(BUILD_TOOLS "Build test/developer tools" ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_compile_options(-Wall -Wextra -Wpedantic)

option(WERROR on)
if (WERROR)
	add_compile_options(-Werror)
endif()

find_package(catkin REQUIRED COMPONENTS dr_error)

catkin_package(
	INCLUDE_DIRS include
	LIBRARIES ${PROJECT_NAME}
	CATKIN_DEPENDS dr_error
)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

find_package(Eigen3)
find_package(yaml-cpp REQUIRED)

add_library(${PROJECT_NAME}
	src/eigen.cpp
	src/error.cpp
	src/types.cpp
	src/yaml.cpp
	src/udp/client.cpp
	src/udp/decode.cpp
	src/udp/encode.cpp
	src/udp/protocol.cpp
	src/rpc_server/rpc_server.cpp
)

include_directories(include/${PROJECT_NAME})
include_directories(SYSTEM ${catkin_INCLUDE_DIRS})

target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC
	${catkin_INCLUDE_DIRS}
	${EIGEN3_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} PUBLIC
	Threads::Threads
	${YAML_CPP_LIBRARIES}
)

catkin_add_gtest(${PROJECT_NAME}_test_yaml src/test/yaml.cpp)
target_link_libraries(${PROJECT_NAME}_test_yaml ${PROJECT_NAME})

install(TARGETS "${PROJECT_NAME}"
	ARCHIVE DESTINATION "${CATKIN_PACKAGE_LIB_DESTINATION}"
	LIBRARY DESTINATION "${CATKIN_PACKAGE_LIB_DESTINATION}"
	RUNTIME DESTINATION "${CATKIN_GLOBAL_BIN_DESTINATION}"
)

install(
	DIRECTORY "include/"
	DESTINATION "${CATKIN_PACKAGE_INCLUDE_DESTINATION}"
)

add_subdirectory(src/file_manager)

if (BUILD_TOOLS)
	add_subdirectory(src/tools)
endif()
