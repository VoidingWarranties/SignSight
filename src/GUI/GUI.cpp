#include <cassert>

#include <iostream>
#include <list>
#include <vector>

#include <opencv2/highgui/highgui.hpp>

#include "GUI/GUI.hpp"


// Displays images from either a specified video device or from images in a specified directory.
// Either video_flag or dir_flag must be true, but NOT both!
// video_index specified the video device.
int displayImageFeed(bool video_dev_flag, int video_index, bool video_file_flag, char* video_path, bool dir_flag, const std::list<char*>& file_paths, cv::Mat (*processingFunction)(cv::Mat&))
{
    // Assertions / assumptions:
    assert((video_dev_flag && !video_file_flag && !dir_flag) ||
           (!video_dev_flag && video_file_flag && !dir_flag) ||
           (!video_dev_flag && !video_file_flag && dir_flag)); // Only one of the flags should be true.

    // The right and left arrow key codes seem to differ from system to system.
    // Change these values as necessary.
    const int RIGHT_ARROW_KEY = 81;
    const int LEFT_ARROW_KEY = 83;

    cv::namedWindow("Output");
    cv::Mat image;

    cv::VideoCapture cam;
    std::list<char*>::const_iterator file_itr;
    if (video_dev_flag) {
        cam.open(video_index);
    } else if (video_file_flag) {
        cam.open(video_path);
    } else if (dir_flag) {
        file_itr = file_paths.begin();
    }

    int wait_amount = 10;
    if (dir_flag) {
        wait_amount = 0;
    }
    while (true) {
        if (video_dev_flag || video_file_flag) {
            cam >> image;
        } else if (dir_flag) {
            image = cv::imread(*file_itr);
        }

        if (! image.data) {
            std::cerr << "Error reading from device or file." << std::endl;
            return 1;
        }

        if (processingFunction) {
            image = processingFunction(image);
        }

        cv::imshow("Output", image);

        char key = cv::waitKey(wait_amount);
        if (key == 'q') {
            break;
        }
        if (dir_flag) {
            if (key == RIGHT_ARROW_KEY) {
                ++file_itr;
                if (file_itr == file_paths.end()) {
                    file_itr = file_paths.begin();
                }
            } else if (key == LEFT_ARROW_KEY) {
                --file_itr;
                if (file_itr == file_paths.end()) {
                    --file_itr;
                }
            }
        }
    }

    return 0;
}
