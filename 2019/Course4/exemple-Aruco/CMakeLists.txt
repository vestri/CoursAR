cmake_minimum_required(VERSION 2.8.1)
project( ARUCO_304_eval )
set(OpenCV_STATIC ON)
find_package( OpenCV REQUIRED )

include_directories( aruco\\include )
link_directories( aruco\\lib )
add_executable( ARUCO_304_eval introduction_windows.cpp)
target_link_libraries( ARUCO_304_eval ${OpenCV_LIBS} aruco304.lib)
