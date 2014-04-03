#ifndef GUI_H_
#define GUI_H_

#include <opencv2/core/core.hpp>

const int ESCAPE_KEY = 27;
const int RIGHT_ARROW_KEY = 81;
const int LEFT_ARROW_KEY = 83;

int displayImageFeed(bool video_dev_flag, int video_dev_index, bool video_file_flag, char* video_file_path, bool dir_flag, const std::list<char*>& file_paths, cv::Mat (*processingFunction)(cv::Mat&) = NULL);

#endif
