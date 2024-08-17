set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_executable(${proj_exe} "main.cpp")

list(APPEND CMAKE_PREFIX_PATH "$ENV{QTDIR}/lib/cmake")
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Widgets Network REQUIRED)       #查找Qt6或Qt5，导入Widgets模块
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Widgets Network Concurrent REQUIRED)  #查找Qt具体版本，导入Widgets模块

list(APPEND CMAKE_PREFIX_PATH "$ENV{OpenCV_Path}/x64/vc15/lib")
find_package(OpenCV REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

target_sources(${proj_exe} PUBLIC "custom_view.cpp" "measure_widget.cpp")

target_link_libraries(${proj_exe} 
	PUBLIC 
		Qt${QT_VERSION_MAJOR}::Core
		Qt${QT_VERSION_MAJOR}::Widgets
		Qt${QT_VERSION_MAJOR}::Concurrent
)