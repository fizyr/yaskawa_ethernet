cmake_minimum_required(VERSION 3.1)
project(yaskawa_ethernet)

option(BUILD_TOOLS "Build test/developer tools" ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
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

# Depencencies
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)
find_package(Eigen3)

# Library
add_library(${PROJECT_NAME}
	src/eigen.cpp
	src/error.cpp
	src/types.cpp
	src/udp/client.cpp
	src/udp/decode.cpp
	src/udp/encode.cpp
	src/udp/protocol.cpp
	src/rpc_server/rpc_server.cpp
)

include_directories(include/${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC
	${catkin_INCLUDE_DIRS}
	${EIGEN3_INCLUDE_DIRS}
	${Boost_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} PUBLIC Threads::Threads ${Boost_LIBRARIES})


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
