add_executable(yaskawa-files file_manager.cpp)
target_link_libraries(yaskawa-files ${PROJECT_NAME})

install(TARGETS yaskawa-files
	RUNTIME DESTINATION "${CATKIN_GLOBAL_BIN_DESTINATION}"
)
