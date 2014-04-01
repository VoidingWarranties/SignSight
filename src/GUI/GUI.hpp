#ifndef GUI_H_
#define GUI_H_

#include <opencv2/core/core.hpp>

int displayImageFeed(bool video_flag, int video_index, bool dir_flag, const std::list<char*>& file_paths, cv::Mat (*processingFunction)(cv::Mat&) = NULL);

#endif
