#include <vector>

#include <opencv2/imgproc/imgproc.hpp>

#include "GUI/Draw.hpp"


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
