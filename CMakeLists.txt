cmake_minimum_required(VERSION 3.1.0)
project(yaskawa-ethernet)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wextra -Wpedantic")

option(WERROR on)
if (WERROR)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
endif()

option(BUILD_SHARED_LIBS "build shared libraries" on)

# Depencencies
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)

# Library
add_library(${PROJECT_NAME}
	src/tcp/client.cpp
	src/tcp/encode.cpp
	src/tcp/decode.cpp
	src/udp/client.cpp
	src/udp/protocol.cpp
	src/error.cpp
)
target_include_directories(${PROJECT_NAME} INTERFACE include PRIVATE include/${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC Threads::Threads ${Boost_LIBRARIES})

# Test executables
add_executable(${PROJECT_NAME}_tcp_test src/tcp_test.cpp)
add_executable(${PROJECT_NAME}_udp_test src/udp_test.cpp)
target_link_libraries(${PROJECT_NAME}_tcp_test ${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME}_udp_test ${PROJECT_NAME})

# Install
include(GNUInstallDirs)

install(TARGETS ${PROJECT_NAME}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
	DIRECTORY include
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
