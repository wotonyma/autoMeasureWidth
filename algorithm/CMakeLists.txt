target_include_directories(${proj_exe} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(${proj_exe} PUBLIC "opencv_test.cpp" "cal_line_width.cpp")

find_package(OpenCV REQUIRED)
target_link_libraries(${proj_exe} PUBLIC "${OpenCV_LIBS}")