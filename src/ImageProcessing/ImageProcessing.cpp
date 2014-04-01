#include <cassert>

#include <opencv2/imgproc/imgproc.hpp>

#include <vector>

#include "ImageProcessing.hpp"

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
