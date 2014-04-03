#include <vector>

#include <opencv2/imgproc/imgproc.hpp>

#include "GUI/Draw.hpp"


void drawBoundingRectangles(cv::Mat& image, const std::vector<std::vector<cv::Point> >& contours, const cv::Scalar& color, const float scale)
{
    for (size_t i = 0; i < contours.size(); ++i) {
        // Scale the rectangle.
        cv::Rect bounding_rect = scaleRectWithLimits(cv::boundingRect(contours[i]), cv::Point(0,0), cv::Point(image.cols,image.rows), scale);
        // Draw the rectangle.
        cv::rectangle(image, bounding_rect.tl(), bounding_rect.br(), color, 2);
    }
}

cv::Rect scaleRectWithLimits(const cv::Rect& source_rect, const cv::Point& tl_limit, const cv::Point& br_limit, float scale)
{
    float position_scale = (scale - 1) / 2;

    int start_x = source_rect.x - (position_scale * source_rect.width);
    int start_y = source_rect.y - (position_scale * source_rect.height);
    int end_x = start_x + (scale * source_rect.width);
    int end_y = start_y + (scale * source_rect.height);

    if (start_x < tl_limit.x)
        start_x = tl_limit.x;
    if (start_y < tl_limit.y)
        start_y = tl_limit.y;
    if (end_x > br_limit.x)
        end_x = br_limit.x;
    if (end_y > br_limit.y)
        end_y = br_limit.y;

    return cv::Rect(start_x, start_y, (end_x - start_x), (end_y - start_y));
}
