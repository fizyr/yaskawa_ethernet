cmake_minimum_required(VERSION 2.8.3)
project(yaskawa_ethernet_client)

find_package(catkin REQUIRED COMPONENTS
	dr_base
)

find_package(Boost REQUIRED COMPONENTS system)

catkin_package(
	INCLUDE_DIRS include
	LIBRARIES ${PROJECT_NAME}
)

dr_include_directories(
	include/${PROJECT_NAME}
	${INCLUDE_DIRS}
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
target_link_libraries(${PROJECT_NAME}_tcp_test ${PROJECT_NAME})
add_executable(${PROJECT_NAME}_udp_test src/udp_test.cpp)
target_link_libraries(${PROJECT_NAME}_udp_test ${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME}
	ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
	LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
	RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)

install(
	DIRECTORY include
	DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION}
)
