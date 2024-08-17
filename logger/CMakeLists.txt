find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
target_link_libraries(${proj_exe} 
	PRIVATE 
		fmt::fmt-header-only
		spdlog::spdlog_header_only
)

SET(log_src "log.cpp" "LogConfigMgr.cpp")

target_include_directories(${proj_exe} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_sources(${proj_exe} PRIVATE ${log_src})


