#include <cassert>

#include <iostream>
#include <list>
#include <vector>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "GUI/GUI.hpp"
#include "GUI/Draw.hpp"
#include "ImageProcessing/ImageProcessing.hpp"


// Displays images from either a specified video device or from images in a specified directory.
// Either video_flag or dir_flag must be true, but NOT both!
// video_index specified the video device.
int displayImageFeed(bool video_flag, int video_index, bool dir_flag, const std::list<char*>& file_paths)
{
    // Assertions / assumptions:
    assert(video_flag ^ dir_flag); // Only video_flag xor dir_flag should be true. Not both!

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
        std::list<char*>::const_iterator file_itr = file_paths.begin();
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
