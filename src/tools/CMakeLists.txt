add_executable(${PROJECT_NAME}_udp_test udp_test.cpp)
target_link_libraries(${PROJECT_NAME}_udp_test ${PROJECT_NAME})

add_executable(${PROJECT_NAME}_udp_read_status udp_read_status.cpp)
target_link_libraries(${PROJECT_NAME}_udp_read_status ${PROJECT_NAME})

install(TARGETS "${PROJECT_NAME}_udp_test" "${PROJECT_NAME}_udp_read_status"
	ARCHIVE DESTINATION "${CATKIN_PACKAGE_LIB_DESTINATION}"
	LIBRARY DESTINATION "${CATKIN_PACKAGE_LIB_DESTINATION}"
	RUNTIME DESTINATION "${CATKIN_GLOBAL_BIN_DESTINATION}"
)
