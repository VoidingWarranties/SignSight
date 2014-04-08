cmake_minimum_required (VERSION 2.8)
project (SignSight)
find_package (OpenCV REQUIRED)

if (UNIX)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
endif(UNIX)

include_directories ("${PROJECT_SOURCE_DIR}/GUI")
add_subdirectory (GUI)

include_directories ("${PROJECT_SOURCE_DIR}/ImageProcessing")
add_subdirectory (ImageProcessing)

add_subdirectory (Tools)

add_executable (SignSight SignSight.cpp)
target_link_libraries (SignSight ${OpenCV_LIBS} GUI Draw ImageProcessing)
