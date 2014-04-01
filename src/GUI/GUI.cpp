#include <unistd.h>
#include <dirent.h>
#include <cstdlib>
#include <cassert>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <list>
#include <vector>
#include <iostream>

#include "GUI/GUI.hpp"
#include "GUI/Draw.hpp"
#include "ImageProcessing/ImageProcessing.hpp"


// Displays images from either a specified video device or from images in a specified directory.
// Either video_flag or dir_flag must be true, but NOT both!
// video_index specified the video device.
// dir_path specified the directory to display images from. This directory should contain only image files.
int displayImageFeed(bool video_flag, int video_index, bool dir_flag, char* dir_path)
{
    // Assertions / assumptions:
    assert(video_flag ^ dir_flag); // Only video_flag xor dir_flag should be true. Not both!
    if (dir_flag) {
        assert(dir_path); // dir_path should not be NULL if dir_flag is true.
    }

    // The right and left arrow key codes seem to differ from system to system.
    // Change these values as necessary.
    const int RIGHT_ARROW_KEY = 65363;
    const int LEFT_ARROW_KEY = 65361;

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

            // Segment the image and draw bounding rectangles for each contour.
            std::vector<std::vector<cv::Point> > contours = segmentForeground(image);
            drawBoundingRectangles(image, contours, cv::Scalar(0,255,0));

            cv::imshow("Output", image);

            int key = cv::waitKey(10);
            if (key == 'q') {
                break;
            }
        }
    } else if (dir_flag) {
        std::list<char*> file_list;
        DIR* dir = opendir(dir_path);
        if (! dir) {
            std::cerr << "Directory " << dir_path << " does not exist!" << std::endl;
            return 1;
        }
        // Iterate over all files in the specified directory.
        dirent* file;
        while ((file = readdir(dir)) != NULL) {
            if (file && file->d_type == DT_REG) {
                // Form the path by concatenating dir_path with each file name.
                char* buffer = new char[strlen(dir_path) + strlen(file->d_name) + 2]();
                strcpy(buffer, dir_path);
                buffer[strlen(dir_path)] = '/';
                strcpy(buffer + strlen(dir_path) + 1, file->d_name);
                // Add the buffer to the end of the doubly linked list.
                file_list.push_back(buffer);
            }
        }
        std::list<char*>::const_iterator file_itr = file_list.begin();
        // Image display loop.
        while (true) {
            image = cv::imread(*file_itr);
            if (! image.data) {
                std::cerr << "Error reading image file " << *file_itr << std::endl;
                return 1;
            }

            // Resize image if it is very large.
            while (image.rows > 1000 || image.cols > 1000) {
                cv::resize(image, image, cv::Size(image.cols / 2, image.rows / 2));
            }

            // Segment the image and draw bounding rectangles for each contour.
            std::vector<std::vector<cv::Point> > contours = segmentForeground(image);
            drawBoundingRectangles(image, contours, cv::Scalar(0,255,0));

            cv::imshow("Output", image);

            int key = cv::waitKey();
            if (key == 'q') {
                break;
            } else if (key == RIGHT_ARROW_KEY) {
                ++file_itr;
                if (file_itr == file_list.end()) {
                    file_itr = file_list.begin();
                }
            } else if (key == LEFT_ARROW_KEY) {
                --file_itr;
                if (file_itr == file_list.end()) {
                    --file_itr;
                }
            }
        }
    }

    return 0;
}
