cmake_minimum_required(VERSION 3.1.0)
project(yaskawa_ethernet_client)

set(CMAKE_CXXFLAGS "${CMAKE_CXXFLAGS} -Wall -Wextra -Wpedantic")

option(WERROR on)
if (WERROR)
	set(CMAKE_CXXFLAGS "${CMAKE_CXXFLAGS} -Werror")
endif()

find_package(Boost REQUIRED COMPONENTS system)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

include_directories(
	include/${PROJECT_NAME}
	SYSTEM ${INCLUDE_DIRS}
)

add_library(${PROJECT_NAME}
	src/tcp/client.cpp
	src/tcp/encode.cpp
	src/tcp/decode.cpp
	src/udp/client.cpp
	src/udp/protocol.cpp
	src/error.cpp
)

target_link_libraries(${PROJECT_NAME} ${catkin_LIBRARIES} ${Boost_LIBRARIES})

add_executable(${PROJECT_NAME}_tcp_test src/tcp_test.cpp)
target_link_libraries(${PROJECT_NAME}_tcp_test Threads::Threads ${PROJECT_NAME})
add_executable(${PROJECT_NAME}_udp_test src/udp_test.cpp)
target_link_libraries(${PROJECT_NAME}_udp_test Threads::Threads ${PROJECT_NAME})

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
