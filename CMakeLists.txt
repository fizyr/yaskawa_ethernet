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
	src/encode.cpp
	src/decode.cpp
	src/client.cpp
)

target_link_libraries(${PROJECT_NAME} ${LIBRARIES})

install(TARGETS ${PROJECT_NAME}
	ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
	LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
	RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)

install(
	DIRECTORY include
	DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION}
)
