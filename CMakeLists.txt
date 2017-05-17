cmake_minimum_required(VERSION 3.1.0)
project(yaskawa_ethernet)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra -Wpedantic)

option(WERROR on)
if (WERROR)
	add_compile_options(-Werror)
endif()

find_package(catkin REQUIRED COMPONENTS dr_error)
catkin_package()

# Depencencies
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)
find_package(Eigen3)

# Library
add_library(${PROJECT_NAME}
	src/tcp/client.cpp
	src/tcp/encode.cpp
	src/tcp/decode.cpp
	src/udp/client.cpp
	src/udp/protocol.cpp
	src/error.cpp
	src/eigen.cpp
)

target_include_directories(${PROJECT_NAME}
	INTERFACE include
	PRIVATE include/${PROJECT_NAME}
)

target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC
	${catkin_INCLUDE_DIRS}
	${EIGEN3_INCLUDE_DIRS}
	${Boost_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} PUBLIC Threads::Threads ${Boost_LIBRARIES})

# Test executables
add_executable(${PROJECT_NAME}_tcp_test src/tcp_test.cpp)
add_executable(${PROJECT_NAME}_udp_test src/udp_test.cpp)
target_link_libraries(${PROJECT_NAME}_tcp_test ${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME}_udp_test ${PROJECT_NAME})

install(TARGETS "${PROJECT_NAME}"
	ARCHIVE DESTINATION "${CATKIN_PACKAGE_LIB_DESTINATION}"
	LIBRARY DESTINATION "${CATKIN_PACKAGE_LIB_DESTINATION}"
	RUNTIME DESTINATION "${CATKIN_GLOBAL_BIN_DESTINATION}"
)

install(
	DIRECTORY "include/"
	DESTINATION "${CATKIN_PACKAGE_INCLUDE_DESTINATION}"
)
