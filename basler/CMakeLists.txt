cmake_minimum_required(VERSION 3.13)

SET(pylon_inc "$ENV{PYLON_DEV_DIR}/include")
SET(pylon_libs_dir "$ENV{PYLON_DEV_DIR}/lib/x64")

SET(basler_src "Basler.cpp" "basler_obj.cpp" "cam_worker.cpp")

message("src = ${basler_src}")

target_sources(${proj_exe} PRIVATE ${basler_src})

target_include_directories(${proj_exe} 
	PUBLIC 
		${pylon_inc}
		${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_directories(${proj_exe} PUBLIC ${pylon_libs_dir})

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Widgets Network REQUIRED)       #查找Qt6或Qt5，导入Widgets模块
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Widgets Network REQUIRED)  #查找Qt具体版本，导入Widgets模块
target_link_libraries(${proj_exe} 
	PUBLIC 
		Qt${QT_VERSION_MAJOR}::Core
		Qt${QT_VERSION_MAJOR}::Widgets
)

