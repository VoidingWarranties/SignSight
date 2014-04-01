#include <unistd.h>
#include <dirent.h>
#include <cstdlib>
#include <cassert>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <list>
#include <vector>
#include <iostream>


int displayImageFeed(bool video_flag, int video_index, bool dir_flag, char* dir_path);
std::vector<std::vector<cv::Point> > segmentForeground(const cv::Mat& image);
void drawBoundingRectangles(cv::Mat& image, const std::vector<std::vector<cv::Point> >& contours, const cv::Scalar& color, const float scale = 1.4);

void print_usage(char* name)
{
    std::cerr << "Usage: " << name << " [-v <video_device_index> | -f <image_directory_path>]"
              << "\n\n"
              << "       If no options are specified video device 0 is used by default."
              << std::endl;
}

int main(int argc, char** argv)
{
    bool video_flag = true;
    int video_index = 0;
    bool dir_flag = false;
    char* dir_path = NULL;

    opterr = 0;
    char c;

    while ((c = getopt(argc, argv, "v:f:")) != -1) {
        switch (c) {
            case 'v':
                video_flag = true;
                dir_flag = false;
                video_index = atoi(optarg);
                break;
            case 'f':
                dir_flag = true;
                video_flag = false;
                dir_path = optarg;
                break;
            case '?':
                print_usage(argv[0]);
                return 1;
                break;
            default:
                return 1;
        }
        if (video_flag && dir_flag) {
            print_usage(argv[0]);
            return 1;
        }
    }

    return displayImageFeed(video_flag, video_index, dir_flag, dir_path);
}

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

// Returns a segmented version of the input image.
// Currently it uses the watershed method for segmentation.
// The input image is expected to have the pixel format BGR. This is the default format when reading from a video feed or image file.
// In the future it would be better if the input image was YUV / YCrCb directly from the camera.
std::vector<std::vector<cv::Point> > segmentForeground(const cv::Mat& image)
{
    // Assertions / assumptions:
    assert(image.rows > 0 && image.cols > 0);

    // First the image's color space is changed from BGR to YCrCb.
    // Then the Y (luma) channel is extracted and stored as a separate Mat object.
    // It is probably more efficient to directly calculate the Y channel from the BGR channels.
    // This might be equivalent to simply converting to gray scale. This should be investigated, as
    // converting directly to gray scale from BGR may be more efficient since it is a built in transformation.
    cv::Mat yuv;
    cv::Mat gray(image.rows, image.cols, CV_8UC1);
    cv::cvtColor(image, yuv, CV_BGR2YCrCb);
    int from_to[] = {0, 0};
    cv::mixChannels(&yuv, 1, &gray, 1, from_to, 1);

    // Gaussian blur to remove noise.
    cv::GaussianBlur(gray, gray, cv::Size(5,5), 0, 0, cv::BORDER_DEFAULT);

    // Detect edges using Canny edge detection.
    int lowThreshold = 100;
    cv::Mat edges;
    cv::Canny(gray, edges, lowThreshold, lowThreshold * 3, 3);

    // Find contours in edge detected image.
    std::vector<std::vector<cv::Point> > initial_contours;
    cv::findContours(edges, initial_contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE); // Consider using CV_CHAIN_APPROX_SIMPLE.

    // Draw the contours and fill in closed contours.
    cv::Mat drawn_contours(image.rows, image.cols, CV_8UC3, cv::Scalar(0, 0, 0));
    for (size_t i = 0; i < initial_contours.size(); ++i) {
        cv::drawContours(drawn_contours, initial_contours, i, cv::Scalar(255), CV_FILLED);
    }
    // Extract just one color layer from the drawn contours.
    // It would be better to draw the contours directly to a CV_8UC1 image, but cv::drawContours seems to exepct a 3 layer image.
    cv::Mat gray_drawn_contours(drawn_contours.rows, drawn_contours.cols, CV_8UC1, cv::Scalar(0));
    cv::mixChannels(&drawn_contours, 1, &gray_drawn_contours, 1, from_to, 1);

    // Apply a distance transform and threshold it.
    // This allows us to only keep contours which have a minimum arbitrary width and height.
    cv::Mat distance_transform;
    cv::distanceTransform(gray_drawn_contours, distance_transform, CV_DIST_L2, CV_DIST_MASK_PRECISE);
    // Normalize the image so that it can be thresholded according to the min or max.
    cv::normalize(distance_transform, distance_transform, 0.0, 1.0, cv::NORM_MINMAX);
    // Threshold.
    cv::Mat foreground;
    cv::threshold(distance_transform, foreground, 0.25, 255, CV_THRESH_BINARY);
    foreground.convertTo(foreground, CV_8UC1);

    // Calculate the contours of the thresholded image.
    std::vector<std::vector<cv::Point> > final_contours;
    cv::findContours(foreground, final_contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);

    return final_contours;
}

void drawBoundingRectangles(cv::Mat& image, const std::vector<std::vector<cv::Point> >& contours, const cv::Scalar& color, const float scale)
{
    const float position_scale = (scale - 1) / 2;
    for (size_t i = 0; i < contours.size(); ++i) {
        // Scale the rectangle.
        cv::Rect bounding_rect = cv::boundingRect(contours[i]);
        bounding_rect.x -= position_scale * bounding_rect.width;
        bounding_rect.y -= position_scale * bounding_rect.height;;
        bounding_rect.width *= scale ;
        bounding_rect.height *= scale ;
        // Draw the rectangle.
        cv::rectangle(image, bounding_rect.tl(), bounding_rect.br(), color, 2);
    }
}
