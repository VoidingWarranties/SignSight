project (ImageSegmenter)

if(UNIX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif(UNIX)

add_executable (ImageSegmenter ImageSegmenter.cpp)
target_link_libraries (ImageSegmenter ${OpenCV_LIBS} ${Boost_LIBRARIES} GUI Draw ImageProcessing)
