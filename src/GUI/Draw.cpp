#include <vector>

#include <opencv2/imgproc/imgproc.hpp>

#include "Draw.hpp"


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
