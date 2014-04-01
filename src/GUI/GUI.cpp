#include <cassert>

#include <iostream>
#include <list>
#include <vector>

#include <opencv2/highgui/highgui.hpp>

#include "GUI/GUI.hpp"


// Displays images from either a specified video device or from images in a specified directory.
// Either video_flag or dir_flag must be true, but NOT both!
// video_index specified the video device.
int displayImageFeed(bool video_flag, int video_index, bool dir_flag, const std::list<char*>& file_paths, cv::Mat (*processingFunction)(cv::Mat&))
{
    // Assertions / assumptions:
    assert(video_flag ^ dir_flag); // Only video_flag xor dir_flag should be true. Not both!

    // The right and left arrow key codes seem to differ from system to system.
    // Change these values as necessary.
    const int RIGHT_ARROW_KEY = 81;
    const int LEFT_ARROW_KEY = 83;

    cv::namedWindow("Output");
    cv::Mat image;

    if (video_flag) {
        cv::VideoCapture cam(video_index);
        // Image display loop.
        while (true) {
            cam >> image;
            if (! image.data) {
                std::cerr << "Error reading camera data. Did someone unplug it?" << std::endl;
            }

            if (processingFunction) {
                image = processingFunction(image);
            }

            cv::imshow("Output", image);

            int key = cv::waitKey(10);
            if (key == 'q') {
                break;
            }
        }
    } else if (dir_flag) {
        std::list<char*>::const_iterator file_itr = file_paths.begin();
        // Image display loop.
        while (true) {
            image = cv::imread(*file_itr);
            if (! image.data) {
                std::cerr << "Error reading image file " << *file_itr << std::endl;
                return 1;
            }

            if (processingFunction) {
                image = processingFunction(image);
            }

            cv::imshow("Output", image);

            char key = cv::waitKey();
            if (key == 'q') {
                break;
            } else if (key == RIGHT_ARROW_KEY) {
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
