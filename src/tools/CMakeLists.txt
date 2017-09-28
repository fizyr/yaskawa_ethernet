add_executable(yaskawa-udp-test udp_test.cpp)
target_link_libraries(yaskawa-udp-test ${PROJECT_NAME})

add_executable(yaskawa-read-status read_status.cpp)
target_link_libraries(yaskawa-read-status ${PROJECT_NAME})

install(TARGETS "yaskawa-udp-test" "yaskawa-read-status"
	ARCHIVE DESTINATION "${CATKIN_PACKAGE_LIB_DESTINATION}"
	LIBRARY DESTINATION "${CATKIN_PACKAGE_LIB_DESTINATION}"
	RUNTIME DESTINATION "${CATKIN_GLOBAL_BIN_DESTINATION}"
)
